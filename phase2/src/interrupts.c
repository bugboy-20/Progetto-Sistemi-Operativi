// Parte di Simone, ma c'ha messo le mandi Diego MUAHAHAH
#include "klog.h"
#include <pandos_utils.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/const.h>
#include <umps3/umps/types.h>
#include <pandos_const.h>
#include <pandos_types.h>
#include <ash.h>
#include <list.h>
#include <pcb.h>
#include <scheduler.h>
#include <systemcall.h>

#define IDBM 0x10000040          // Interrupt Devices BitMap
#define TERMINALBM (IDBM + 0x10) // Interrupt Line 7 Interrupting Devices Bit Map
#define PRINTERBM (IDBM + 0x0C)  // Interrupt Line 6 Interrupting Devices Bit Map
#define NETWORKBM (IDBM + 0x08)  // Interrupt Line 5 Interrupting Devices Bit Map
#define FLASHBM (IDBM + 0x04)    // Interrupt Line 4 Interrupting Devices Bit Map
#define DISKBM (IDBM + 0x00)     // Interrupt Line 3 Interrupting Devices Bit Map

#define RT_STATUS 0xFF

// calculate the address of the device's device register
/*inline memaddr devAddrBase(int IntlineNo, int DevNo) {
    return 0x10000054 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10);
}*/

#define devAddrBase(IntlineNo, DevNo) (memaddr)(0x10000054 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10))

static inline int getDevNo(unsigned int *bitMap_address)
{
    unsigned int bitMap = *bitMap_address;
    for (char i = 0; i < 8; i++)
        if ((1 << i) & bitMap)
            return i;

    return -1; // Error
}

void dtpInterruptHandler(int IntlineNo, int DevNo)
{
    // Calculate the address for this device's register
    dtpreg_t *dev = (dtpreg_t *)devAddrBase(IntlineNo, DevNo);
    // Save off the status code from the device’s device register
    unsigned int status = dev->status;
    // Acknowledge the outstanding interrupt. This is accomplished by writing the acknowledge command code in the interrupting device’s device register
    dev->command = ACK;
    // Perform a V operation on the Nucleus maintained semaphore associated with this (sub)device. This operation should unblock the process (pcb) which initiated this I/O operation and then requested to wait for its completion via a SYS5 operation.
    int *devSem = (int *)dev_sem_addr(IntlineNo, DevNo);
    pcb_t *proc = headBlocked(devSem);
    V(devSem);
    // If this process is present
    if (proc != NULL)
    {
        // Place the stored off status code in the newly unblocked pcb’s v0 register.
        proc->p_s.reg_v0 = status;
        // Insert the newly unblocked pcb on the Ready Queue, transitioning this process from the “blocked” state to the “ready” state.
        // Already done by the Verhogen
    }
    // else
    // {
    //     // Verhogen decrement `soft_block_count` only if process != NULL, this needs to be decremented always 'cause of the DOIO syscall
    //     soft_block_count -= 1;
    // }
    if (current_proc != NULL)
    {
        // TODO: controllare che effettivamente vada fatto STCK prima di ogni LDST
        STCK(start_time);
        LDST(EXCEPTION_STATE); // Return control to the Current Process: Perform a LDST on the saved exception state
    }
    else
        scheduler(); // Call the scheduler;
}

void termInterruptHandler(int IntlineNo, int DevNo)
{
    unsigned int status;
    int *devSem;
    // Calculate the address for this device's register
    termreg_t *term = &(((devreg_t *)devAddrBase(IntlineNo, DevNo))->term);
    if ((term->transm_status & RT_STATUS) > READY)
    {
        // Save off the status code from the device’s device register
        status = term->transm_status & 0xF;
        // Acknowledge the outstanding interrupt. This is accomplished by writing the acknowledge command code in the interrupting device’s device register
        term->transm_command = ACK;
        // Perform a V operation on the Nucleus maintained semaphore associated with this (sub)device. This operation should unblock the process (pcb) which initiated this I/O operation and then requested to wait for its completion via a SYS5 operation.
        devSem = (int *)dev_sem_addr(IntlineNo, DevNo);
    }
    else if ((term->recv_status & RT_STATUS) > READY)
    {
        // Save off the status code from the device’s device register
        status = term->recv_status;
        // Acknowledge the outstanding interrupt. This is accomplished by writing the acknowledge command code in the interrupting device’s device register
        term->recv_command = ACK;
        // Perform a V operation on the Nucleus maintained semaphore associated with this (sub)device. This operation should unblock the process (pcb) which initiated this I/O operation and then requested to wait for its completion via a SYS5 operation.
        devSem = (int *)dev_sem_addr((IntlineNo + 1), DevNo);
    }

    pcb_t *proc = headBlocked(devSem);
    V(devSem);
    // If this process is present
    if (proc != NULL)
    {
        // Place the stored off status code in the newly unblocked pcb’s v0 register.
        proc->p_s.reg_v0 = 0;
        value_bak[0] = status;

        // Insert the newly unblocked pcb on the Ready Queue, transitioning this process from the “blocked” state to the “ready” state.
        // insertProcQ(&ready_q, proc);
    }
    // else
    // {
    //     // Verhogen decrement `soft_block_count` only if process != NULL, this needs to be decremented always 'cause of the DOIO syscall
    //     soft_block_count -= 1;
    // }
    if (current_proc != NULL)
    {
        // TODO: controllare che effettivamente vada fatto STCK prima di ogni LDST
        STCK(start_time);
        LDST(EXCEPTION_STATE); // Return control to the Current Process: Perform a LDST on the saved exception state
    }
    else
        scheduler(); // Call the scheduler;
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
void interrupt_handler()
{
    unsigned int cause = EXCEPTION_STATE->cause; // Custom system for getting cause
    // klog_print("Interrupt cause: ");
    // klog_print_hex(cause);
    // klog_print("\n");

    // ignoring interrupt line 0

    // PLT exception, line 1
    if (cause & LOCALTIMERINT)
    {
        // Acknowledge the PLT interrupt, and load the timer again
        // TODO: Controllare se caricare il PLT con un valore enorme sia corretto, invece di caricarlo con il valore di 100ms, l'output non cambia al momento
        // setTIMER(TIMESLICE * (*((cpu_t *)TIMESCALEADDR)));
        setTIMER(__UINT32_MAX__);
        // Copy the processor state in the current process status
        current_proc->p_s = *EXCEPTION_STATE;
        // Save time of process before transition running -> ready
        time_now(end_time);
        current_proc->p_time += (end_time - start_time);
        // Move the process from running to ready
        insertProcQ(&ready_q, current_proc);
        // Call the scheduler
        scheduler();
    }
    // Interval Timer exception, line 2
    else if (cause & TIMERINTERRUPT)
    {
        // Load the Interval Timer with 100ms
        LDIT(PSECOND);
        // Unblock ALL pcbs blocked on the pseudo-clock semaphore
        pcb_t *proc;
        while ((proc = removeBlocked(&pseudoclock_semaphore)) != NULL)
        {
            insertProcQ(&ready_q, proc);
            soft_block_count -= 1;
        }

        // Reset the Pseudo-clock semaphore to 0
        pseudoclock_semaphore = 0;
        // Return control to the Current Process with LDST
        // If not present return to the scheduler, which will do WAIT() or HALT()
        if (current_proc != NULL)
        {
        // TODO: controllare che effettivamente vada fatto STCK prima di ogni LDST
            STCK(start_time);
            LDST(EXCEPTION_STATE);
        }
        else
            scheduler();
    }
    // Line 3-7, these ar the devices
    else if (cause & DISKINTERRUPT)
        dtpInterruptHandler(DISKINT, getDevNo((memaddr *)DISKBM));
    else if (cause & FLASHINTERRUPT)
        dtpInterruptHandler(FLASHINT, getDevNo((memaddr *)FLASHBM));
    else if (cause & PRINTINTERRUPT)
        dtpInterruptHandler(PRNTINT, getDevNo((memaddr *)PRINTERBM));
    else if (cause & TERMINTERRUPT)
        termInterruptHandler(TERMINT, getDevNo((memaddr *)TERMINALBM));
}
