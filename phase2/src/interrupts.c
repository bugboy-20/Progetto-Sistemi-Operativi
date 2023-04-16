// Parte di Simone, ma c'ha messo le mandi Diego MUAHAHAH
#include "umps/const.h"
#include <pandos_types.h>
#include <pandos_const.h>
#include <umps/types.h>
void notTimeInterrupt();
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
    state_t *state = (memaddr) BIOSDATAPAGE;
    unsigned int cause = ((state_t*) BIOSDATAPAGE)->cause;


    // ignoring interrupt line 0 
    /* When there are multiple interrupts pending, and the interrupt exception han-
dler processes only the single highest priority pending interrupt, the interrupt
exception handler will be immediately re-entered as soon as interrupts are
unmasked again; effectively forming a loop until all the pending interrupts
are processed.
*///TODO capire cosa intende con "...as soon as interrupts are unmasked again; "


    if (cause & TIMERINTERRUPT);
    if (cause & DISKINTERRUPT);
    if (cause & FLASHINTERRUPT);
    if (cause & PRINTINTERRUPT);
    if (cause & TERMINTERRUPT);
}

void notTimeInterrupt() {
    /*1. Calculate the address for this device’s device register. [Section ??-pops]
2. Save off the status code from the device’s device register.
3. Acknowledge the outstanding interrupt. This is accomplished by writ-
ing the acknowledge command code in the interrupting device’s device
register.
Alternatively, writing a new command in the interrupting
device’s device register will also acknowledge the interrupt.
4. Perform a V operation on the Nucleus maintained semaphore associ-
ated with this (sub)device. This operation should unblock the process
(pcb) which initiated this I/O operation and then requested to wait for
its completion via a SYS5 operation.
5. Place the stored off status code in the newly unblocked pcb’s v0 register.
6. Insert the newly unblocked pcb on the Ready Queue, transitioning this
process from the “blocked” state to the “ready” state.
7. Return control to the Current Process: Perform a LDST on the saved
exception state (located at the start of the BIOS Data Page [Section
     * */


}
