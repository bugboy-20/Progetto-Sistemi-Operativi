#include <exceptions.h>
#include <pandos_utils.h>
#include <interrupts.h>
#include <systemcall.h>
#include <pandos_const.h>
#include <pandos_types.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <scheduler.h>

// Pass Up or Die function, if supportStruct is NULL kill the process, otherwise pass up it
HIDDEN void pass_up_or_die(int exep_code);

// Handler for system calls
HIDDEN void syscall_handler(unsigned int, unsigned int, unsigned int, unsigned int);

void exception_handler()
{
    const int exception_code = CAUSE_GET_EXCCODE(EXCEPTION_STATE->cause);

    switch (exception_code)
    {
    case EXC_INT:
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
        syscall_handler(EXCEPTION_STATE->reg_a0, EXCEPTION_STATE->reg_a1, EXCEPTION_STATE->reg_a2, EXCEPTION_STATE->reg_a3);
        break;

    default:
        // call to program trap exception handler or else
        pass_up_or_die(GENERALEXCEPT);
        break;
    }
}

HIDDEN void syscall_handler(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3)
{
    /** controllare se siamo in kernel mode,
     * se no e` necessario lanciare una trap
     * inoltre se il processo usa un codice non valido deve essere terminato
     * capitolo 3.5.9 e 3.7
     */
    if ((EXCEPTION_STATE->status & STATUS_KUp) >> STATUS_KUp_BIT == 1)
    {
        // process is in user mode then trigger program trap exception
        // clean cause.ExcCode
        EXCEPTION_STATE->cause &= ~CAUSE_EXCCODE_MASK;
        // set the ExcCode to Reserved Instruction
        EXCEPTION_STATE->cause |= EXC_RI << CAUSE_EXCCODE_BIT;
        pass_up_or_die(GENERALEXCEPT);
        return;
    }

    switch (a0)
    {
    case CREATEPROCESS:
        create_process((state_t *)a1, (struct support_t *)a2, (nsd_t *)a3);
        break;
    case TERMPROCESS:
        terminate_process((int)a1);
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
        get_process_id((bool)a1);
        break;
    case GETCHILDREN:
        get_children((int *)a1, (int)a2);
        break;
    default:
        pass_up_or_die(GENERALEXCEPT);
        break;
    }
}

HIDDEN void pass_up_or_die(int exep_code)
{
    if (current_proc->p_supportStruct == NULL)
        // no support struct, terminate current process
        terminate_process(0);
    else
    {
        // handle the pass up
        current_proc->p_supportStruct->sup_exceptState[exep_code] = *EXCEPTION_STATE;
        context_t exep_context = current_proc->p_supportStruct->sup_exceptContext[exep_code];
        LDCXT(exep_context.stackPtr, exep_context.status, exep_context.pc);
    }
}
