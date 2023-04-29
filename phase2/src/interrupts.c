// Parte di Simone, ma c'ha messo le mandi Diego MUAHAHAH
#include <umps/cp0.h>
#include <umps/libumps.h>
#include <syscall.h>
#include <umps/const.h>
#include <umps/types.h>
#include <pandos_const.h>
#include <pandos_types.h>
#include <ash.h>
#include <initial.h>
#include <list.h>
#include <pcb.h>
#include <scheduler.h>

#define IDBM 0x10000040             // Interrupt Devices BitMap
#define TERMINALBM  IDBM + 0x10     // Interrupt Line 7 Interrupting Devices Bit Map
#define PRINTERBM   IDBM + 0x0C     // Interrupt Line 6 Interrupting Devices Bit Map
#define NETWORKBM   IDBM + 0x08     // Interrupt Line 5 Interrupting Devices Bit Map
#define FLASHBM     IDBM + 0x04     // Interrupt Line 4 Interrupting Devices Bit Map
#define DISKBM      IDBM + 0x00     // Interrupt Line 3 Interrupting Devices Bit Map

// calculate the address of the device's device register
inline memaddr devAddrBase(int IntlineNo, int DevNo) {
    return 0x10000054 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10);
}

inline int getDevNo(const int bitMap) {
    for(char i = 0; i < 8; i++) 
        if(1 << i & bitMap)
            return i;
    
    return -1; //Error
}

void notTimerInterrupt(int IntlineNo, int DevNo) {
    // Calculate the address for this device's register
    dtpreg_t *dev = (dtpreg_t*) devAddrBase(IntlineNo, DevNo);
    // Save off the status code from the device’s device register
    unsigned int status = dev->status;
    // Acknowledge the outstanding interrupt. This is accomplished by writing the acknowledge command code in the interrupting device’s device register
    dev->command = ACK;
    // Perform a V operation on the Nucleus maintained semaphore associated with this (sub)device. This operation should unblock the process (pcb) which initiated this I/O operation and then requested to wait for its completion via a SYS5 operation.
    int* devSem = (int*) dev_sem_addr(IntlineNo, DevNo);
    pcb_t* proc = headBlocked(devSem);
    verhogen(devSem);
    // If this process is present
    if (proc != NULL) {
        // Place the stored off status code in the newly unblocked pcb’s v0 register.
        proc->p_s.reg_v0 = status;
        // Insert the newly unblocked pcb on the Ready Queue, transitioning this process from the “blocked” state to the “ready” state.
        insertProcQ(&ready_q, proc);
    }
    // Return control to the Current Process: Perform a LDST on the saved exception state
    state_t *state = EXCEPTION_STATE;
    LDST(state);

}

/*
 * Interrupt Line   |   Device Class
 *  -----------------------------------------------
 *              0   |   Inter-processor interrupts
 *              1   |   Processor Local Timer
 *              2   |   Bus (Interval Timer)
 *              3   |   Disk Devices
 *              4   |   Tape Devices
 *              5   |   Network (Ethernet) Devices
 *              6   |   Printer Devices
 *              7   |   Terminal Devices
 */
void interrupt_handler() {
    unsigned int cause = EXCEPTION_STATE->cause; // Custom system for getting cause

    // ignoring interrupt line 0 

    // PLT exception, line 1
    if (cause & LOCALTIMERINT) {
        // Acknowledge the PLT interrupt, and load the timer again
        setTIMER(TIMESLICE * (*((cpu_t*) TIMESCALEADDR)));
        // Copy the processor state in the current process status
        current_proc->p_s.status = EXCEPTION_STATE->status;
        // Move the process from running to ready
        insertProcQ(&ready_q, current_proc);
        // Call the scheduler
        scheduler();
    }
    // Interval Timer exception, line 2
    else if (cause & TIMERINTERRUPT) {
        // Load the Interval Timer with 100ms
        LDIT(PSECOND);
        // Unblock ALL pcbs blocked on the pseudo-clock semaphore
        pcb_t *proc;
        while ((proc = removeBlocked(&pseudoclock_semaphore)) != NULL)
        {
            // TODO: direi che devo metterli nella ready q?
            insertProcQ(&ready_q, proc);
            soft_block_count -= 1;
        }

        // Reset the Pseudo-clock semaphore to 0
        pseudoclock_semaphore = 0;
        // Return control to the Current Process with LDST
        // If not present return to the scheduler, which will do WAIT() or HALT()
        if (current_proc != NULL)
            LDST(EXCEPTION_STATE);
        else
            scheduler();

    }
    // Line 3-7, these ar the devices
    else {
        if (cause & DISKINTERRUPT)
            notTimerInterrupt(3, getDevNo(DISKBM));
        if (cause & FLASHINTERRUPT)
            notTimerInterrupt(4, getDevNo(FLASHBM));
        if (cause & PRINTINTERRUPT)
            notTimerInterrupt(5, getDevNo(PRINTERBM));
        if (cause & TERMINTERRUPT);
    }
}
