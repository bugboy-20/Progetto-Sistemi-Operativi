#include "../include/syscall.h"
#include <umps3/umps/libumps.h>
#include <types.h>
#include <pcb.h>
#include <ash.h>
#include <initial.h>
#include <scheduler.h>

HIDDEN void terminate_recursively(pcb_t *);
HIDDEN void syscall_end(bool terminated, bool blocking);
HIDDEN pcb_t *get_proc(int pid);

void create_process(state_t *statep, struct support_t *supportp, nsd_t *ns)
{
    state_t *state = (state_t *)BIOSDATAPAGE;
    pcb_t *new_proc = allocPcb();
    if (new_proc == NULL)
    {
        state->reg_v0 = -1;
        return;
    }

    new_proc->p_supportStruct = supportp;
    // pid is the address of the pcb itself
    new_proc->p_pid = (unsigned int)new_proc;

    // new_proc->p_s = *statep;
    new_proc->p_time = 0;
    new_proc->p_semAdd = NULL;

    // insert new process into the reade queue
    insertProcQ(ready_q, new_proc);

    // new process is the progeny of the caller
    insertChild(current_proc, new_proc);

    // se ns è NULL dovrei usare il namespace del padre
    if (ns == NULL)
        ns = current_proc->namespaces[NS_PID];
    new_proc->namespaces[NS_PID] = ns;

    process_count += 1;

    // TODO: metti il pid nel registro v0
    state->reg_v0 = new_proc->p_pid;

    syscall_end(false, false);
}

void terminate_process(int pid)
{
    if (pid == 0)
    {
        // termino il processo corrente e tutta la sua progenie
        terminate_recursively(current_proc);
        syscall_end(true, false);
        return;
    }
    pcb_t *dead_proc = get_proc(pid);
    terminate_recursively(dead_proc);

    syscall_end(false, false);
}

// questa syscall e` bloccante, capitolo 3.5.11
void passeren(int *semAddr)
{
    // current process goes from running to blocked
    if (*semAddr == 0)
    {
        insertBlocked(semAddr, current_proc);
        current_proc == NULL;
        // TODO: incrementare il counter dei processi bloccati
        //  schedule the next process
        syscall_end(false, true);
    }
    // unblock the first blocked process
    else
    {
        pcb_t *proc = removeBlocked(semAddr);
        if (proc == NULL)
        {
            // no proc is blocked, resource given to current proc
            (*semAddr) -= 1;
            syscall_end(false, false);
        }
        else
        {
            insertProcQ(ready_q, proc);
            // TODO: decrementare il counter dei processi bloccati
            syscall_end(false, true);
        }
    }
}

void verhogen(int *semAddr)
{
    if (*semAddr = 1)
    {
        // la V non si può fare se il sem è a 1
        insertBlocked(semAddr, current_proc);
        current_proc == NULL;
        // TODO: incrementare il counter dei processi bloccati
        //  schedule the next process
        syscall_end(false, true);
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
            insertProcQ(ready_q, proc);
            // TODO: decrementare il counter dei processi bloccati
        }
    }
}

// questa syscall e` bloccante, capitolo 3.5.11
void do_io(int *cmdAddr, int *cmdValues)
{
    // istruzioni non chiare, non so come implementarla

    // la richiesta di IO blocca sempre il processo
    // TODO: capire qual'è il semaforo sul quale il processo deve bloccarsi
    // insertBlocked(..., current_proc);
    current_proc == NULL;
    scheduler();

    // TODO: capire se questa è l'operazione giusta da fare
    cmdAddr = cmdValues;
}

void get_cpu_time() {}

// questa syscall e` bloccante, capitolo 3.5.11
void wait_for_clock() {}

void get_support_data() {}

void get_process_id(bool parent) {}

void get_children(int *children, int size) {}

HIDDEN void syscall_end(bool terminated, bool blocking)
{
    if (terminated)
    {
        scheduler();
        return;
    }
    state_t *state = (state_t *)BIOSDATAPAGE;

    state->pc_epc += WORDLEN;
    if (blocking)
    {
        // TODO: verificare se l'assegnamento effettua una copia vera e propria
        // current_proc->p_s = *state;
        // TODO: aggiornare il CPU time per il processo corrente
        // TODO: capire come fare a fare la transition da uno stato ready a blocked
        scheduler();
    }
    else
    {
        LDST(state);
        // TODO: capire se devo chiamare lo scheduler
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

    process_count -= 1;

    if (proc == current_proc)
    {
        // running state, don't have to do anything (?)
    }
    else if (outProcQ(ready_q, proc) == NULL) // ready state
    {
        // blocked state
        outBlocked(proc);
        // TOOD: aggiustare il soft-block count se necessario
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

    list_for_each_entry(tmp, ready_q, p_list)
    {
        if (tmp->p_pid == pid)
        {
            return tmp;
        }
    }
    return NULL;
}