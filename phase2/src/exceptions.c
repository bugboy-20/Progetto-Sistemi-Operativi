#include <pandos_utils.h>
#include <exceptions.h>
#include <interrupts.h>
#include <pandos_const.h>
#include <pandos_types.h>
#include <systemcall.h>
#include <klog.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <scheduler.h>

void exception_handler()
{
    const int exception_code = CAUSE_GET_EXCCODE(EXCEPTION_STATE->cause);
    klog_print("Exception code: ");
    klog_print_hex(exception_code);
    klog_print("\n");

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
        // WORKAROUND per evitare di chiamare la terminate_process
        pass_up_or_die(GENERALEXCEPT);
        //scheduler();
        break;
    }
}

void syscall_handler(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3)
{
    // klog_print("Dentro syscallhandler\n");
    // klog_print("A0: ");
    // klog_print_hex(a0);
    // klog_print("\n");
    // klog_print("A1: ");
    // klog_print_hex(a1);
    // klog_print("\n");
    // klog_print("A2: ");
    // klog_print_hex(a2);
    // klog_print("\n");
    // klog_print("A3: ");
    // klog_print_hex(a3);
    // klog_print("\n");
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
        terminate_process((int)a1);
        break;
    case PASSEREN:
        // klog_print("Caso passeren in syscall handler\n");
        passeren((int *)a1);
        break;
    case VERHOGEN:
        // klog_print("Caso verhogen in syscall handler\n");
        verhogen((int *)a1);
        break;
    case DOIO:
        // klog_print("Caso do_io in syscall handler\n");
        do_io((unsigned int*) a1, (unsigned int) a2);
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
        get_process_id((bool)a1);
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
    klog_print("Dentro pass_up_or_die");
    if (current_proc->p_supportStruct == NULL)
    {
        klog_print(" -> current_proc->p_supportStruct == NULL, sto per fare terminate_process\n");
        terminate_process(0);
    }
    else
    {
        klog_print(" -> current_proc->p_supportStruct != NULL");
        current_proc->p_supportStruct->sup_exceptState[exep_code] = *EXCEPTION_STATE;
        context_t exep_context = current_proc->p_supportStruct->sup_exceptContext[exep_code];
        LDCXT(exep_context.stackPtr, exep_context.status, exep_context.pc);
    }
}
