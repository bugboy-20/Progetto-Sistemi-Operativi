#ifndef INITIAL_H
#define INITIAL_H
#include <pandos_types.h>
#define  EXCEPTION_STATE ((state_t*)BIOSDATAPAGE)

typedef struct{} sem_t; //TODO

extern passupvector_t* pass_up_vector;
extern int process_count;
extern int soft_block_count;
extern pcb_PTR current_proc;
extern struct list_head* ready_q;
extern sem_t sem_table[DEVICECNT];
extern cpu_t start_time;




// Returns the address of the semaphore related to a specific device
// type : type of devise use DISKINT, FLASHINT, ecc... (defined in umps/const.h)
#define dev_sem_addr(type, n) ((memaddr) &sem_table[(type -3)*DEVPERINT + n])


#endif //INITIAL_H
