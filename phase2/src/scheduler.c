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
        // All bit set to zero (PLT disabled) OR Interrupts Enabled OR Interrupt Mask full ON, with this the PLT became disabled
        // klog_print("Waiting...\n");
        // If WAIT is awakened by something but the situation is the same we recall WAIT
        // TODO: prova se cambiando la Interrupt Mask funziona, altrimenti lasciare un while
        while (process_count > 0 && soft_block_count > 0)
        {
            setSTATUS(ALLOFF | IECON | IMON);
            WAIT();
        }
    }
    else if (process_count > 0 && soft_block_count == 0)
    {
        // Deadlock
        PANIC();
    }

}
