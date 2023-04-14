// Parte di Simone, ma c'ha messo le mandi Diego MUAHAHAH
#include "umps/const.h"
#include <pandos_types.h>
#include <pandos_const.h>
#include <umps/types.h>
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
    //state_t *state = (memaddr) BIOSDATAPAGE;
    unsigned int cause = ((state_t*) BIOSDATAPAGE)->cause;


    if (cause & TIMERINTERRUPT);
    if (cause & DISKINTERRUPT);
    if (cause & FLASHINTERRUPT);
    if (cause & PRINTINTERRUPT);
    if (cause & TERMINTERRUPT);
    }
