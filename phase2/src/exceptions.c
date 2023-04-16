#include <exceptions.h>
#include <initial.h>
#include <interrupts.h>
#include <pandos_const.h>
#include <pandos_types.h>
#include "../include/syscall.h"
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>

void exception_handler()
{
    state_t *state = (state_t *)(state_t *)BIOSDATAPAGE;
    const int exception_code = CAUSE_GET_EXCCODE(state->cause);

    switch (exception_code)
    {
    case EXC_INT:
        // TODO: chiamare l'interrupt handler
        interrupt_handler();
        break;
    case EXC_MOD:
    case EXC_TLBL:
    case EXC_TLBS:
        // call to tlb exception handler
        pass_up_or_die(PGFAULTEXCEPT);
        break;

        // case EXC_ADEL:
        // case EXC_ADES:
        // case EXC_IBE:
        // case EXC_DBE:
        // case EXC_BP:
        // case EXC_RI:
        // case EXC_CPU:
        // case EXC_OV:
        //     pass_up_or_die(GENERALEXCEPT);
        //     break;

    case EXC_SYS:
        // call to syscall excpetion handler
        syscall_handler(state->reg_a0, &state->reg_a1, &state->reg_a2, &state->reg_a3);
        break;

    default:
        // call to program trap exception handler or else
        pass_up_or_die(GENERALEXCEPT);
        break;
    }
}

void syscall_handler(int a0, void *a1, void *a2, void *a3)
{
    state_t *state = (state_t *)BIOSDATAPAGE;

    if ((state->status & STATUS_KUp) >> STATUS_KUp_BIT == 1)
    {
        // process is in user mode then trigger program trap exception
        // clean cause.ExcCode
        state->cause &= ~CAUSE_EXCCODE_MASK;
        // set the ExcCode to Reserved Instruction
        state->cause |= EXC_RI << CAUSE_EXCCODE_BIT;
        pass_up_or_die(GENERALEXCEPT);
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
        create_process((state_t *)a1, (struct support_t *)a2, (nsd_t *)a3);
        break;
    case TERMPROCESS:
        terminate_process(*(int *)a1);
        break;
    case PASSEREN:
        passeren((int *)a1);
        break;
    case VERHOGEN:
        verhogen((int *)a1);
        break;
    case DOIO:
        do_io((int *)a1, (int *)a2);
        break;
    // Michele
    case GETTIME:
        get_cpu_time();
        break;
    case CLOCKWAIT:
        wait_for_clock();
        break;
    case GETSUPPORTPTR:
        get_support_data();
        break;
    case GETPROCESSID:
        get_process_id(*(bool *)a1);
        break;
    case GETCHILDREN:
        get_children((int *)a1, *(int *)a2);
        break;
    default:
        pass_up_or_die(GENERALEXCEPT);
        break;
    }
}

void pass_up_or_die(int exep_code)
{
    if (current_proc->p_supportStruct == NULL)
    {
        terminate_process(current_proc->p_pid);
    }
    else
    {
        state_t *state = (state_t *)BIOSDATAPAGE;
        // current_proc->p_supportStruct->sup_exceptState[exep_code] = *state;
        context_t exep_context = current_proc->p_supportStruct->sup_exceptContext[exep_code];
        LDCXT(exep_context.stackPtr, exep_context.status, exep_context.pc);
    }
}
