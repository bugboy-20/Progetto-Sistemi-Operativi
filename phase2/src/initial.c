#include <ash.h>
#include <pcb.h>
#include <ns.h>
#include <pandos_utils.h>
#include <scheduler.h>
#include <exceptions.h>
#include <umps3/umps/libumps.h>
#include <pandos_const.h>

extern void test();
extern void uTLB_RefillHandler();

passupvector_t* pass_up_vector;
int process_count;
int soft_block_count;
pcb_PTR current_proc;
struct list_head ready_q;
int sem_table[NUMSEM];
int pseudoclock_semaphore;
cpu_t start_time;
int *value_bak;

int main() {
    // Inizialize the Pass Up Vector
    pass_up_vector = (passupvector_t*) PASSUPVECTOR;
    pass_up_vector->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    pass_up_vector->tlb_refill_stackPtr = (memaddr) KERNELSTACK;
    pass_up_vector->exception_handler = (memaddr) exception_handler;
    pass_up_vector->exception_stackPtr = (memaddr) KERNELSTACK;

    // Initialize Phase 2 data structures
    initPcbs();
    initASH();
    initNamespaces();
    
    // Inizialize all nucleus maintained variables
    process_count=1;
    soft_block_count=0;
    mkEmptyProcQ(&ready_q);
    current_proc = NULL;
    for (int i = 0; i < NUMSEM; i++)
        sem_table[i] = 1;
    pseudoclock_semaphore = 0;

    // Set Interval Timer to 100ms = 100000ps
    LDIT(PSECOND);

    // Instantiate the init process
    pcb_t* init = allocPcb();
    // Enable interrupts (IEPON), enable processor Local Timer (TEBITON), kernel mode on
    // IEPON = Interrupts Enable Previous ON
    // TEBITON = Timer Enable BIT ON
    // USERPON = USER Process ON, we need the process in Kernel mode, so we use 0 as baseline
    init->p_s.status = 0 | IEPON | TEBITON;
    // SP set to RAMTOP, reg_sp is a macro for accessing the bit tied to the stack pointer
    RAMTOP(init->p_s.reg_sp);
    // PC set to the address of `test`
    init->p_s.pc_epc = (memaddr) test;
    // We have to assign the same address to reg_t9
    init->p_s.reg_t9 = (memaddr) test;
    // Set all Process Tree fields to NULL
    init->p_parent = NULL;
    mkEmptyProcQ(&init->p_child);
    mkEmptyProcQ(&init->p_sib);
    // Set the accumulated time field to zero
    init->p_time = 0;
    // Set the blocking semaphore address to NULL
    init->p_semAdd = NULL;
    // Set the Support Structure pointer to NULL
    init->p_supportStruct = NULL;

    // Insert the process in the ready queue and call the scheduler
    insertProcQ(&ready_q, init);
    scheduler();

    return 0;
}

