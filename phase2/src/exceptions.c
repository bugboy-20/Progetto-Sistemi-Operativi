// Parte di Pische e Michele
#include <exceptions.h>
#include <pandos_const.h>
#include <pandos_types.h>
#include "syscall.h"
#include <cp0.h>

// costanti temporanee
// finchè non abbiamo i file del prof
#define GETPROCESSID 9
#define GET_CHILDREN 10

memaddr exceptionHandler()
{
    state_t *state = BIOSDATAPAGE;
    const int exception_code = CAUSE_GET_EXCCODE(state->cause);

    switch (exception_code)
    {
    case EXC_INT:
        // TODO: chiamare l'interrupt handler
        break;
    case EXC_MOD:
    case EXC_TLBL:
    case EXC_TLBS:
        // TODO: chiamare tlb exception handler
        break;

    case EXC_ADEL:
    case EXC_ADES:
    case EXC_IBE:
    case EXC_DBE:
    case EXC_BP:
    case EXC_RI:
    case EXC_CPU:
    case EXC_OV:
        // TODO: chiamare program trap exception handler
        break;

    case EXC_SYS:
        // TODO: chiamare il syscall excpetion handler
        break;

    default:
        break;
    }
    return 0;
}

int SYSCALL(int a0, void *a1, void *a2, void *a3)
{
    state_t *state = BIOSDATAPAGE;
    int *KUp = (state->status & STATUS_KUp) >> STATUS_KUp_BIT;

    if (KUp == 1)
    {
        // process is in user mode
        // clean cause.ExcCode
        state->cause &= ~CAUSE_EXCCODE_MASK;
        // set the ExcCode to Reserved Instruction
        state->cause |= EXC_RI << CAUSE_EXCCODE_BIT;
        // TODO: chiamata al trap handler
        return;
    }
    /** controllare se siamo in kernel mode,
     * se no e` necessario lanciare una trap
     * inoltre se il processo usa un codice non valido deve essere terminato
     * capitolo 3.5.9 e 3.7
     */
    switch (a0)
    {
    // Pische
    case CREATEPROCESS:
        return create_process((state_t *)a1, (struct support_t *)a2, (nsd_t *)a3);
    case TERMPROCESS:
        terminate_process(*(int *)a1);
        break;
    case PASSEREN:
        passeren((int *)a1);
        break;
    case VERHOGEN:
        verhogen((int *)a1);
        break;
    case IOWAIT:
        return do_io((int *)a1, (int *)a2);
    // Michele
    case GETTIME:
        return get_cpu_time();
    case CLOCKWAIT:
        return wait_for_clock();
    case GETSUPPORTPTR:
        return get_support_data();
    case GETPROCESSID:
        return get_process_id(*(int *)a1);
    case GET_CHILDREN:
        return get_children((int *)a1, *(int *)a2);
    default:
        // TODO: terminare il processo che ha fatto la syscall sbagliata
        return -1;
    }
    /**
     * prima di terminare dovrei aumentare il valore del PC di 4
     * capitolo 3.5.10 del manuale
     */
    // ho assunto che il valore di ritorno per le syscall void sia 0

    state->pc_epc += WORDLEN;
    return 0;
}
