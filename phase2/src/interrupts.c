// Parte di Simone, ma c'ha messo le mandi Diego MUAHAHAH
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

#define IDBM 0x10000040 // Interrupt Devices BitMap
#define TERMINALBM  0x10000040 + 0x10 //Interrupt Line 7 Interrupting Devices Bit Map
#define PRINTERBM   0x10000040 + 0x0C //Interrupt Line 6 Interrupting Devices Bit Map
#define NETWORKBM   0x10000040 + 0x08 //Interrupt Line 5 Interrupting Devices Bit Map
#define FLASHBM     0x10000040 + 0x04 //Interrupt Line 4 Interrupting Devices Bit Map
#define DISKBM      0x10000040 + 0x00//Interrupt Line 3 Interrupting Devices Bit Map

// calculate the device's device register
inline memaddr deviceReg(int IntlineNo, int DevNo) {
    return 0x10000054 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10);
}

inline int getDevNo(const int bitMap) {
    for(char i=0; i<8; i++) 
        if(1<<i & bitMap)
            return i;
    
    return -1; //Error
}

void dtpInterrupt(int IntlineNo, int DevNo) {
    /*
        6. Insert the newly unblocked pcb on the Ready Queue, transitioning this
        process from the “blocked” state to the “ready” state.
        7. Return control to the Current Process: Perform a LDST on the saved
        exception state (located at the start of the BIOS Data Page [Section
     * */
    dtpreg_t *dev = (dtpreg_t*) deviceReg(IntlineNo, DevNo);
    int* devSem = (int*) dev_sem_addr(IntlineNo, DevNo);
    unsigned int status = dev->status;
    dev->command = ACK;
    pcb_t* proc = headBlocked(devSem);
    verhogen(devSem);
    proc->p_s.reg_v0 = status;

    //TODO aggiungere a ready_q e cambiare stato del processo
    // list_add_tail(&proc->p_list, ready_q);
    insertProcQ(ready_q, proc);
    
    

    //TODO LDST(void *statep);

    //TODO controllare che sia corretto
    state_t *state = (memaddr) BIOSDATAPAGE;
    LDST(state);

}

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
    state_t *state = EXCEPTION_STATE; // TODO Controllare se effettivamente serve
    unsigned int cause = EXCEPTION_STATE->cause;


    // ignoring interrupt line 0 
    /* When there are multiple interrupts pending, and the interrupt exception han-
dler processes only the single highest priority pending interrupt, the interrupt
exception handler will be immediately re-entered as soon as interrupts are
unmasked again; effectively forming a loop until all the pending interrupts
are processed.
*///TODO capire cosa intende con "...as soon as interrupts are unmasked again; "


    if (cause & TIMERINTERRUPT) {
        setTIMER(TIMESLICE * (*((cpu_t*) TIMESCALEADDR)));
        current_proc->p_s.status = EXCEPTION_STATE->status;
        insertProcQ(ready_q, current_proc);
        scheduler();
    }
    if (cause & DISKINTERRUPT)
        dtpInterrupt(3, getDevNo(DISKBM));
    if (cause & FLASHINTERRUPT)
        dtpInterrupt(4, getDevNo(FLASHBM));
    if (cause & PRINTINTERRUPT)
        dtpInterrupt(5, getDevNo(PRINTERBM));
    if (cause & TERMINTERRUPT);
}
