#ifndef INITIAL_H
#define INITIAL_H
#include "pandos_types.h"

typedef struct{} sem_t; //TODO

passupvector_t* pass_up_vector;
int process_count;
int soft_block_count;
pcb_PTR current_proc;
struct list_head* ready_q;
sem_t sem_table[DEVICECNT];

#endif //INITIAL_H
