#include "../include/syscall.h"
#include <pcb.h>
#include <ash.h>
#include <TODO.h>

int create_process(state_t *statep, struct support_t *supportp, nsd_t *ns)
{
    pcb_t *new_proc = allocPcb();
    if (new_proc == NULL)
        return -1;

    // TODO: se ns è NULL dovrei usare il namespace del padre
    if (ns == NULL)
        ns = processo_attuale->namespaces[NS_PID];

    // TODO: aggiungere la struttura di supporto al pcb
    // TODO: aggiungere il pid nel pcb

    new_proc->p_s = *statep;
    new_proc->p_time = 0;
    new_proc->p_semAdd = NULL;

    // TODO: inserire il pbc nella ready queue
    insertProcQ(readyQ, new_proc);

    // TODO: bisogna rendere new_proc come figlio del chiamante
    insertChild(processo_attuale, new_proc);
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
    bool failure = insertBlocked(semAddr, processo_attuale);
    if (!failure)
    {
        // TODO: inserisco il processo nelle coda dei bloccati?
        // TODO: chiamo lo scheduler
        scheduler();
    }
    else
    {
        // TODO: lascio il controllo al processo corrente
    }
}

void verhogen(int *semAddr)
{
    pcb_t *proc = removeBlocked(semAddr);
    // TODO: forse devo fare operazioni/controlli in più?
}

// questa syscall e` bloccante, capitolo 3.5.11
int do_io(int *cmdAddr, int *cmdValues)
{
    // istruzione nella slide non chiare e manuale non aggiornato
    // non so come implementarla
    return -1;
}

int get_cpu_time();

// questa syscall e` bloccante, capitolo 3.5.11
int wait_for_clock();

struct support_t *get_support_data();

int get_process_id(int parent);

int get_children(int *children, int size);
