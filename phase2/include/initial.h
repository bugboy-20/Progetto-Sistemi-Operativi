#ifndef INITIAL_H
#define INITIAL_H
#include <pandos_types.h>

typedef struct{} sem_t; //TODO

extern passupvector_t* pass_up_vector;
extern int process_count;
extern int soft_block_count;
extern pcb_PTR current_proc;
extern struct list_head* ready_q;
extern sem_t sem_table[DEVICECNT];

#endif //INITIAL_H
