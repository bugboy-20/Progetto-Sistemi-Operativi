#include <systemcall.h>
#include <umps3/umps/libumps.h>
#include <types.h>
#include <pcb.h>
#include <ash.h>
#include <ns.h>
#include <pandos_utils.h>
#include <scheduler.h>
#include <pandos_const.h>

#define BLOCKING true
#define TERMINATED true

HIDDEN void terminate_recursively(pcb_t *);
HIDDEN void syscall_end(bool terminated, bool blocking);
HIDDEN pcb_t *get_proc(int pid);

static devregarea_t *devregarea = (devregarea_t *)RAMBASEADDR;

void create_process(state_t *statep, struct support_t *supportp, nsd_t *ns)
{
    pcb_t *new_proc = allocPcb();
    if (new_proc == NULL)
    {
        EXCEPTION_STATE->reg_v0 = -1;
        syscall_end(!TERMINATED, !BLOCKING);
        return;
    }

    new_proc->p_supportStruct = supportp;
    // pid is the address of the pcb itself
    new_proc->p_pid = (unsigned int)new_proc;

    new_proc->p_s = *statep;
    new_proc->p_time = 0;
    new_proc->p_semAdd = NULL;

    // insert new process into the reade queue
    insertProcQ(&ready_q, new_proc);

    // new process is the progeny of the caller
    insertChild(current_proc, new_proc);

    // if ns is NULL we use parent namespace
    if (ns == NULL)
        ns = current_proc->namespaces[NS_PID];
    new_proc->namespaces[NS_PID] = ns;

    process_count += 1;

    EXCEPTION_STATE->reg_v0 = new_proc->p_pid;

    syscall_end(!TERMINATED, !BLOCKING);
}

void terminate_process(int pid)
{
    terminate_recursively(get_proc(pid));
    if (current_proc != NULL)
    {
        // Get end time
        time_now(end_time);
        // Update process time before transition running -> ready
        current_proc->p_time += (end_time - start_time);
        insertProcQ(&ready_q, current_proc);
    }
    syscall_end(TERMINATED, !BLOCKING);
}

// This system is blocking, Section 3.5.11
void passeren(int *semAddr)
{
    bool is_blocking = P(semAddr);
    syscall_end(!TERMINATED, is_blocking);
}

// This system is blocking, Section 3.5.11
void verhogen(int *semAddr)
{
    bool is_blocking = V(semAddr);
    syscall_end(!TERMINATED, is_blocking);
}

// This system is blocking, Section 3.5.11
void do_io(int *cmdAddr, int *cmdValues)
{
    /**
     * cmdValues is an array of 2 elements for the terminal devices,
     * 4 elements for all the other devices
     */
    int type = getTypeDevice(cmdAddr);
    int n_dev = getNumDevice(cmdAddr);

    int *command = &cmdAddr[1];
    value_bak = cmdValues;

    switch (type)
    {
    case 3 ... 6:
        // Case for Normal Devices
        // IO request is always blocking
        int *devSemAddr = (int *)dev_sem_addr(type, n_dev);
        *devSemAddr = 0; // forcing the P call to be blocking
        P(devSemAddr);
        *command = cmdValues[1];
        break;

    case 7:
        // Case for Terminals
        // if it's in transmission mode
        if (&devregarea->devreg[type - 3][n_dev].term.transm_command == (unsigned int *)command)
        {
            // IO request is always blocking
            int *devSemAddr = (int *)dev_sem_addr(type, n_dev);
            *devSemAddr = 0; // forcing the P call to be blocking
            P(devSemAddr);

            *command = cmdValues[1];
        }
        // else it's in receve mode
        else if (&devregarea->devreg[type - 3][n_dev].term.recv_command == (unsigned int *)command)
        {
            // IO request is always blocking
            int *devSemAddr = (int *)dev_sem_addr(type + 1, n_dev);
            *devSemAddr = 0; // forcing the P call to be blocking
            P(devSemAddr);

            *command = cmdValues[1];
        }
        break;
    
    default:
        // Type error, return -1
        current_proc->p_s.reg_v0 = -1;
        syscall_end(!TERMINATED, BLOCKING);
        break;
    }
    
    // All ok, return 0 and call syscall_end
    current_proc->p_s.reg_v0 = 0;
    syscall_end(!TERMINATED, BLOCKING);
}

void get_cpu_time()
{
    time_now(end_time);
    EXCEPTION_STATE->reg_v0 = current_proc->p_time + (end_time - start_time);
    syscall_end(!TERMINATED, !BLOCKING);
}

// This system is blocking, Section 3.5.11
void wait_for_clock()
{
    P(&pseudoclock_semaphore);
    syscall_end(!TERMINATED, BLOCKING);
}

void get_support_data()
{
    EXCEPTION_STATE->reg_v0 = (memaddr)current_proc->p_supportStruct;
    syscall_end(!TERMINATED, !BLOCKING);
}

void get_process_id(bool parent)
{
    if (parent)
    {
        if (getNamespace(current_proc, NS_PID) != getNamespace(current_proc->p_parent, NS_PID))
            EXCEPTION_STATE->reg_v0 = 0;
        else
            EXCEPTION_STATE->reg_v0 = current_proc->p_parent->p_pid;
    }
    else
        EXCEPTION_STATE->reg_v0 = current_proc->p_pid;
    syscall_end(!TERMINATED, !BLOCKING);
}

void get_children(int *children, int size)
{
    int counter = 0;
    pcb_t *i;
    list_for_each_entry(i, &current_proc->p_child, p_sib)
    {
        if (getNamespace(current_proc, NS_PID) == getNamespace(i, NS_PID))
        {
            if (children != NULL && counter < size)
                children[counter] = i->p_pid;
            counter++;
        }
    }
    EXCEPTION_STATE->reg_v0 = counter;
    syscall_end(!TERMINATED, !BLOCKING);
}

HIDDEN void syscall_end(bool terminated, bool blocking)
{
    if (terminated)
    {
        current_proc = NULL;
        scheduler();
        return;
    }

    EXCEPTION_STATE->pc_epc += WORDLEN;
    if (blocking)
    {
        // The blocking system calls have a P in them, so the process is already blocked, we just need to save the current_proc state and time
        current_proc->p_s = *EXCEPTION_STATE;
        time_now(end_time);
        current_proc->p_time += (end_time - start_time);
        current_proc = NULL;
        scheduler();
    }
    else
        LDST(EXCEPTION_STATE);
}

HIDDEN void terminate_recursively(pcb_t *proc)
{
    /**
     * manuale al capitolo 3.9
     */
    while (!emptyChild(proc))
        terminate_recursively(removeChild(proc));

    if (proc->p_pid == current_proc->p_pid)
    {
        // running state, kill the current process
        current_proc = NULL;
    }
    else if (outProcQ(&ready_q, proc) == NULL) // ready state
    {
        // blocked state
        if (outBlocked(proc) != NULL)
        {
            soft_block_count -= 1;
        }
    }
    process_count -= 1;
    outChild(proc);
    freePcb(proc);
}

HIDDEN pcb_t *get_proc(int pid)
{
    // The pid of our process is their address, so (pcb_t *)pid
    if (pid == 0)
        return current_proc;
    else // pid != 0
        return (pcb_t *)pid;
}
