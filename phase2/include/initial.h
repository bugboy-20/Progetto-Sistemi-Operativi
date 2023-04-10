#ifndef INITIAL_H
#define INITIAL_H
#include <pandos_types.h>
typedef struct{} sem_t; //TODO

int process_count;
int soft_block_count;
pcb_PTR current_proc;
LIST_HEAD(ready_q);
sem_t sem_table[MAXPROC];

#endif //INITIAL_H
