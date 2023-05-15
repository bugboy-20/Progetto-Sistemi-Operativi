#include <pandos_utils.h>
#include <pcb.h>
#include <klog.h>
#include <scheduler.h>
#include <umps/const.h>
#include <umps/libumps.h>

void scheduler()
{
    // klog_print("Scheduler");
    // KLOG_PRETTI_PRINT("Process count: ", process_count);
    // KLOG_PRETTI_PRINT("Soft block: ", soft_block_count);
    if (!emptyProcQ(&ready_q))
    {
        // klog_print("not empty ready_q\n");
        current_proc = removeProcQ(&ready_q);
        // TIMESLICE = 5000, time is in ps (1 ms = 1000 ps)
        setTIMER(TIMESLICE * (*((cpu_t*) TIMESCALEADDR)));
        LDST(&(current_proc->p_s));
    }
    else if (process_count == 0)
    {
        // Successfully termination
        HALT();
    }
    else if (process_count > 0 && soft_block_count > 0)
    {
        // Entering a Wait State
        // All bit set to zero (PLT disabled) OR Interrupts Enabled OR Interrupt Mask full ON, with this the PLT became disabled
        // klog_print("Waiting...\n");
        setSTATUS(ALLOFF | IECON | IMON);
        WAIT();
    }
    else if (process_count > 0 && soft_block_count == 0)
    {
        // Deadlock
        PANIC();
    }

}
