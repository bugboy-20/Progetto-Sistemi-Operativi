#include <systemcall.h>
#include <umps3/umps/libumps.h>
#include <types.h>
#include <pcb.h>
#include <ash.h>
#include <ns.h>
#include <pandos_utils.h>
#include <scheduler.h>
#include <pandos_const.h>
#include <klog.h>

#define BLOCKING true
#define TERMINATED true

HIDDEN void terminate_recursively(pcb_t *);
HIDDEN void syscall_end(bool terminated, bool blocking);
HIDDEN pcb_t *get_proc(int pid);

static devregarea_t *devregarea = (devregarea_t *)RAMBASEADDR;

void P(int *semAddr)
{
    // current process goes from running to blocked
    if (*semAddr == 0)
    {
        // TODO: rimuovere current_proc dalla ready_q?
        insertBlocked(semAddr, current_proc);
        // current_proc = NULL;
        soft_block_count += 1;
    }
    // unblock the first blocked process
    else
    {
        pcb_t *proc = removeBlocked(semAddr);
        if (proc == NULL)
        {
            // no other proc is blocked, resource given to current proc
            (*semAddr) -= 1;
        }
        else
        {
            insertProcQ(&ready_q, proc);
            soft_block_count -= 1;
        }
    }
}

void V(int *semAddr)
{
    if (*semAddr == 1)
    {
        // when semAddr = 1, V is blocking
        insertBlocked(semAddr, current_proc);
        current_proc = NULL;
        soft_block_count += 1;
    }
    else
    {
        pcb_t *proc = removeBlocked(semAddr);
        if (proc == NULL)
        {
            // no proc is blocked, resource given to current proc
            (*semAddr) += 1;
        }
        else
        {
            insertProcQ(&ready_q, proc);
            soft_block_count -= 1;
        }
    }
}

// TODO capire dove metterla
// Copia `state_t` struct
void copy_state(state_t *dest, const state_t *source)
{
    klog_print("Inizio copia di state_t\n");
    dest->entry_hi = source->entry_hi;
    klog_print("<->");
    dest->cause = source->cause;
    klog_print("<->");
    dest->status = source->status;
    klog_print("<->");
    dest->pc_epc = source->pc_epc;
    klog_print("<->");
    for (int i = 0; i < STATE_GPR_LEN; i++)
    {
        dest->gpr[i] = source->gpr[i];
    }
    klog_print("<->");
    dest->hi = source->hi;
    klog_print("<->");
    dest->lo = source->lo;
    klog_print("Fine copia di state_t\n");
}

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
    if (pid == 0 || current_proc->p_pid == pid)
    {
        // termino il processo corrente e tutta la sua progenie
        terminate_recursively(current_proc);
        syscall_end(TERMINATED, !BLOCKING);
        return;
    }
    pcb_t *dead_proc = get_proc(pid);
    terminate_recursively(dead_proc);

        syscall_end(!TERMINATED, !BLOCKING);
}

// questa syscall e` bloccante, capitolo 3.5.11
void passeren(int *semAddr)
{
    klog_print("passeren->");
    // current process goes from running to blocked
    if (*semAddr == 0)
    {
        klog_print("*semAddr==0\n");
        // TODO: rimuovere current_proc dalla ready_q?
        insertBlocked(semAddr, current_proc);
        current_proc = NULL;
        soft_block_count += 1;
        //  schedule the next process
        syscall_end(!TERMINATED, BLOCKING);
    }
    // unblock the first blocked process
    else
    {
        klog_print("*semAddr!=0");
        pcb_t *proc = removeBlocked(semAddr);
        if (proc == NULL)
        {
            klog_print("->proc==NULL\n");
            // no other proc is blocked, resource given to current proc
            (*semAddr) -= 1;
            syscall_end(!TERMINATED, !BLOCKING);
        }
        else
        {
            klog_print("->proc!=NULL\n");
            insertProcQ(&ready_q, proc);
            soft_block_count -= 1;
            syscall_end(!TERMINATED, BLOCKING);
        }
    }
}

void verhogen(int *semAddr)
{
    klog_print("semAddr: ");
    klog_print_hex(*semAddr);
    klog_print("\n");
    if (*semAddr == 1)
    {
        // when semAddr = 1, V is blocking
        insertBlocked(semAddr, current_proc);
        current_proc = NULL;
        soft_block_count += 1;
        //  schedule the next process
        syscall_end(!TERMINATED, BLOCKING);
    }
    else
    {
        pcb_t *proc = removeBlocked(semAddr);
        if (proc == NULL)
        {
            // no proc is blocked, resource given to current proc
            (*semAddr) += 1;
            klog_print("semAddr incrementato: ");
            klog_print_hex(*semAddr);
            klog_print("\n");
            syscall_end(!TERMINATED, !BLOCKING);
        }
        else
        {
            insertProcQ(&ready_q, proc);
            soft_block_count -= 1;
            syscall_end(!TERMINATED, BLOCKING);
        }
    }
}

// questa syscall e` bloccante, capitolo 3.5.11
void do_io(int *cmdAddr, int *cmdValues)
{
    klog_print("Dentro do_io\n");
    // istruzioni non chiare, non so come implementarla

    /**
     * cmdValues is an array of 2 elements for the terminal devices,
     * 4 elements for all the other devices
     */
    /*
    // ((address - startaddress) / register size) + device starting index
    int type = ((*cmdAddr - 0x10000054) / 0x80) + 3;
    //((*cmdAddr - startaddress) - ((type - 3) * register size)) / device n size;
    int n = ((*cmdAddr - 0x10000054) - ((type - 3) * 0x80)) / 0x10;

    // la richiesta di IO blocca sempre il processo
    insertBlocked((int *)dev_sem_addr(type, n), current_proc);
    klog_print("Inserito processo nei bloccati\n");
    current_proc = NULL;
    soft_block_count += 1;

    // TODO: capire se questa è l'operazione giusta da fare
    *cmdAddr = cmdValues;
    klog_print("doio finita\n");
    syscall_end(!TERMINATED, BLOCKING);
    */
    int *status = &cmdAddr[0];
    int *command = &cmdAddr[1];
    value_bak = cmdValues;


    for (int n = 0; n < DEVPERINT; n++)
    {
        // klog_print_hex(cmdAddr);
        // klog_print("\n");
        // klog_print_hex(&devregarea->devreg[4][n].term.transm_command);
        // klog_print("\n");
        // klog_print_hex(&devregarea->devreg[4][n].term.recv_command);
        // klog_print("\n");
        // Terminals are the 4th device
        if (&devregarea->devreg[4][n].term.transm_command == (unsigned int *)command)
        {
            klog_print("DENTRO IF TRANSMISSIONE");
            klog_print(cmdValues);
            klog_print_hex(cmdValues);

            // la richiesta di IO blocca sempre il processo
            int *devSemAddr = (int *)dev_sem_addr(7, n);
            P(devSemAddr);

            *command = cmdValues[1];
            klog_print("assegnamento cmdvalues fatto\n");
        }
        if (&devregarea->devreg[4][n].term.recv_command == (unsigned int *)command)
        {
            klog_print("DENTRO IF RICEZIONE");
            klog_print(cmdValues);
            klog_print_hex(cmdValues);

            // la richiesta di IO blocca sempre il processo
            int *devSemAddr = (int *)dev_sem_addr(7 + 1, n);
            P(devSemAddr);

            *command = cmdValues[1];
            klog_print("assegnamento cmdvalues fatto\n");
        }
    }
    klog_print("Fine do_io\n");
    current_proc->p_s.reg_v0 = 0;
    syscall_end(!TERMINATED, BLOCKING);
}

void get_cpu_time()
{
    unsigned int end_time;
    STCK(end_time);
    EXCEPTION_STATE->reg_v0 = current_proc->p_time + (end_time - start_time);
    syscall_end(!TERMINATED, !BLOCKING);
}

// questa syscall e` bloccante, capitolo 3.5.11
void wait_for_clock()
{
    passeren(&pseudoclock_semaphore);
    // syscall_end(!TERMINATED, BLOCKING);
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
            if (counter < size)
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
        current_proc=NULL;
        scheduler();
        return;
    }

    EXCEPTION_STATE->pc_epc += WORDLEN;
    if (blocking)
    {
        klog_print("Prima di assegnamento");
        // TODO con questa istruzione viene dato una eccezione 6
        //copy_state(&current_proc->p_s, state);
        current_proc->p_s = *EXCEPTION_STATE;
        klog_print("Dopo di assegnamento");
        current_proc=NULL;
        // TODO: aggiornare il CPU time per il processo corrente
        // TODO: capire come fare a fare la transition da uno stato ready a blocked
        scheduler();
    }
    else
    {
        LDST(EXCEPTION_STATE);
    }
}

HIDDEN void terminate_recursively(pcb_t *proc)
{
    /**
     * manuale al capitolo 3.9
     */
    if (proc == NULL)
        return;
    outChild(proc);

    // TODO: incrementare il semaforo giusto se necessario

    // WORKAROUND patch per farlo stampare
    process_count -= 1;

    if (proc == current_proc)
    {
        freePcb(proc);
        current_proc = NULL;
        // running state, don't have to do anything (?)
    }
    else if (outProcQ(&ready_q, proc) == NULL) // ready state
    {
        // blocked state
        outBlocked(proc);
        // TODO: aggiustare il soft-block count se necessario
        soft_block_count -= 1;
    }

    struct pcb_t *tmp;

    list_for_each_entry(tmp, &proc->p_child, p_sib)
    {
        terminate_recursively(tmp);
    }
}

HIDDEN pcb_t *get_proc(int pid)
{
    if (current_proc->p_pid == pid)
        return current_proc;

    struct pcb_t *tmp;

    list_for_each_entry(tmp, &ready_q, p_list)
    {
        if (tmp->p_pid == pid)
        {
            return tmp;
        }
    }
    return NULL;
}
