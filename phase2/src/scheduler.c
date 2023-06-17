#include <pandos_utils.h>
#include <pcb.h>
#include <klog.h>
#include <scheduler.h>
#include <umps/const.h>
#include <umps/libumps.h>

void scheduler()
{
    if (!emptyProcQ(&ready_q))
    {
        current_proc = removeProcQ(&ready_q);
        // TIMESLICE = 5000, time is in ps (1 ms = 1000 ps)
        setTIMER(TIMESLICE * (*((cpu_t*) TIMESCALEADDR)));
        // start_time = TOD timer
        STCK(start_time);
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
        // The Status register need the interrupt bit enabled, and the interrupt mask fully activated
        // except the bit corrisponding to the lines 1 (PLT) and 2 (Interval Timer)
        // Reference: uMPS3 Principles of Operation 7.2.2
        setSTATUS(STATUS_MASK_FOR_WAIT);
        WAIT();
    }
    else if (process_count > 0 && soft_block_count == 0)
    {
        // Deadlock
        PANIC();
    }

}
