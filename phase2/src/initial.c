// Parte di Diego

#include "../include/ash.h"
#include "../include/pcb.h"
#include "../include/ns.h"
#include "../include/initial.h"
#include "../include/scheduler.h"
#include "list.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include <umps/const.h>
#include <umps/libumps.h>

extern void test();
extern void uTLB_RefillHandler();

int main() {
    // TODO
    // INIZIALIZZAZIONE SCHEDULER

    /*
     * Inizializzazione: scheduler
        - Allocare un processo (pcb_t) in kernel mode, con interrupt abilitati, stack pointer a RAMTOP e program counter che punta alla funzione test() (fornita da noi).
        - Inserire questo processo nella Ready Queue.
        - invocare lo scheduler
    */

    // Inizialize the Pass Up Vector
    pass_up_vector = PASSUPVECTOR;
    pass_up_vector->tlb_refll_handler = (memaddr) uTLB_RefillHandler;
    pass_up_vector->tlb_refll_stackPtr = (memaddr) KERNELSTACK;
    pass_up_vector->exception_handler = (memaddr) exceptionHandler;
    pass_up_vector->exception_stackPtr = (memaddr) KERNELSTACK;

    // Initialize Phase 2 data structures
    initPcbs();
    initASH();
    initNamespaces();
    
    // Inizialize all nucleus maintained variables
    process_count=0;
    soft_block_count=0;
    ready_q = mkEmptyProcQ();
    current_proc = NULL;

    // Set Interval Timer to 100ms
    LDIT(100);

    // Instantiate the init process
    pcb_t init = allocPcb();
    // TODO enable interrupts
    // TODO enable processor Local Timer
    // TODO kernel mode on
    // TODO SP set to RAMTOP
    // TODO PC set to the address of `test`
    init.p_list = NULL;
    init.p_child = NULL;
    init.p_parent = NULL;
    init.p_child = NULL;
    init.p_sib = NULL;
    init.p_time = 0;
    init.p_semAdd = NULL;
    init.p_supportStruct = NULL; // TODO definire support struct in qualche modo

    /*
    Important Point: When setting up a new processor state one must set the previous bits (i.e. IEp & KUp) and not the current bits (i.e. IEc & KUc) in the Status register for the desired assignment to take effect after the initial LDST loads the processor state. [Section ??-pops]
    */

    /*
    Test is a supplied function/process that will help you debug your Nucleus. One can assign a variable (i.e. the PC) the address of a function by using
        yyy->p_s.s_pc = (memaddr) test;

    For rather technical reasons, whenever one assigns a value to the PC one must also assign the same value to the general purpose register t9. (a.k.a. s_t9 as defined in types.h.) [Section ??-pops]
    */

    scheduler();

    return 0;
}
