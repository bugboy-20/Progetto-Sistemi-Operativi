#include <pandos_utils.h>

#define BLOCKING true

void *memcpy(void *dest, const void * src, size_t n) {
    char byte;
    for(int i=0; i<n; i++) {
        byte = *((char*) src+i);
        *((char*) dest+i) = byte;

    }
    return dest;
}


bool P(int *semAddr)
{
    // current process goes from running to blocked
    if (*semAddr == 0)
    {
        // TODO: rimuovere current_proc dalla ready_q?
        insertBlocked(semAddr, current_proc);
        soft_block_count += 1;
        return BLOCKING;
    }
    // unblock the first blocked process
    else
    {
        pcb_t *proc = removeBlocked(semAddr);
        if (proc == NULL)
        {
            // no other proc is blocked, resource given to current proc
            (*semAddr) -= 1;
            return !BLOCKING;
        }
        else
        {
            insertProcQ(&ready_q, proc);
            soft_block_count -= 1;
            return BLOCKING;
        }
    }
}

bool V(int *semAddr)
{
    if (*semAddr == 1)
    {
        // when semAddr = 1, V is blocking
        insertBlocked(semAddr, current_proc);
        soft_block_count += 1;
        return BLOCKING;
    }
    else
    {
        pcb_t *proc = removeBlocked(semAddr);
        if (proc == NULL)
        {
            // no proc is blocked, resource given to current proc
            (*semAddr) += 1;
            return !BLOCKING;
        }
        else
        {
            insertProcQ(&ready_q, proc);
            soft_block_count -= 1;
            return BLOCKING;
        }
    }
}

#undef BLOCKING