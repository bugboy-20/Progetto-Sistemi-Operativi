#include "../include/syscall.h"
#include <pcb.h>
#include <ash.h>
#include <TODO.h>

int create_process(state_t *statep, struct support_t *supportp, nsd_t *ns)
{
    pcb_t *new_proc = allocPcb();
    if (new_proc == NULL)
        return -1;

    // TODO: aggiungere la struttura di supporto al pcb
    // new_proc.p_supportStruct = supportp;
    // TODO: aggiungere il pid nel pcb, uso l'indirizzo del pcb
    // new_proc.p_pid = new_proc;

    new_proc->p_s = *statep;
    new_proc->p_time = 0;
    new_proc->p_semAdd = NULL;

    // TODO: inserire il pbc nella ready queue
    insertProcQ(readyQ, new_proc);

    // TODO: bisogna rendere new_proc come figlio del chiamante
    insertChild(processo_attuale, new_proc);

    // TODO: se ns è NULL dovrei usare il namespace del padre
    if (ns == NULL)
        ns = processo_attuale->namespaces[NS_PID];
    new_proc->namespaces[NS_PID] = ns;

    // TODO: incrementare il counter dei processi attuali
    // proc_count+=1;
    return 0;
}

void terminate_process(int pid)
{
    if (pid == 0)
    {
        // termino il processo corrente e tutta la sua progenie
        terminate_recursively(processo_attuale);
        return;
    }
    pcb_t *dead_proc = get_proc_by_pid(pid);
    terminate_recursively(dead_proc);

    scheduler();
}

// questa syscall e` bloccante, capitolo 3.5.11
void passeren(int *semAddr)
{
    // current process goes from running to blocked
    if (*semAddr == 0)
    {
        insertBlocked(semAddr, processo_attuale);
        processo_attuale == NULL;
        // TODO: incrementare il counter dei processi bloccati
        //  schedule the next process
        scheduler();
    }
    // unblock the first blocked process
    else
    {
        pcb_t *proc = removeBlocked(semAddr);
        if (proc == NULL)
        {
            // no proc is blocked, resource given to current proc
            (*semAddr) -= 1;
        }
        else
        {
            insertProcQ(readyQ, proc);
            // TODO: decrementare il counter dei processi bloccati
        }
    }
}

void verhogen(int *semAddr)
{
    if ((*semAddr) = 1)
    {
        // la V non si può fare se il sem è a 1
        insertBlocked(semAddr, processo_attuale);
        processo_attuale == NULL;
        // TODO: incrementare il counter dei processi bloccati
        //  schedule the next process
        scheduler();
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
            insertProcQ(readyQ, proc);
            // TODO: decrementare il counter dei processi bloccati
        }
    }
}

// questa syscall e` bloccante, capitolo 3.5.11
int do_io(int *cmdAddr, int *cmdValues)
{
    // istruzioni non chiare, non so come implementarla

    // la richiesta di IO blocca sempre il processo
    // TODO: capire qual'è il semaforo sul quale il processo deve bloccarsi
    // insertBlocked(..., processo_attuale);
    processo_attuale == NULL;
    scheduler();

    // TODO: capire se questa è l'operazione giusta da fare
    (*cmdAddr) = cmdValues;
    return -1;
}

int get_cpu_time();

// questa syscall e` bloccante, capitolo 3.5.11
int wait_for_clock();

struct support_t *get_support_data();

int get_process_id(int parent);

int get_children(int *children, int size);
