#ifndef UTILS_H
#define UTILS_H
// funzioni e macro utili un po' ovunque

#include <pandos_const.h>
#include <pandos_types.h>

#define NUMSEM (DEVICECNT + 8)

extern passupvector_t *pass_up_vector;
extern int process_count;
extern int soft_block_count;
extern pcb_PTR current_proc;
extern struct list_head ready_q;
extern int sem_table[NUMSEM];
extern int pseudoclock_semaphore;
extern cpu_t start_time;
extern int *value_bak;

// Returns the address of the semaphore related to a specific device
// type : type of device use DISKINT, FLASHINT, ecc... (defined in umps/const.h)
#define dev_sem_addr(type, n) ((memaddr)&sem_table[(type - 3) * DEVPERINT + n])

#define EXCEPTION_STATE ((state_t *)BIOSDATAPAGE)

typedef int size_t;
// copy a memory portion
void *memcpy(void *dest, const void *src, size_t n);

// Definition of binary semaphore operation
bool P(int *);
bool V(int *);

#define time_now(var) \
    cpu_t var;        \
    STCK(var);

#define STATUS_MASK_FOR_WAIT ((IECON | IMON) & 0xFFFFFCFF)

// Macros for choosing the correct device, used in DOIO
// ((address - startaddress) / register size) + device starting index
#define getTypeDevice(cmdAddr) (((int)cmdAddr - 0x10000054) / 0x80 + 3)
//((*cmdAddr - startaddress) - ((type - 3) * register size)) / device n size;
#define getNumDevice(cmdAddr) ((((int)cmdAddr - 0x10000054) - ((getTypeDevice(cmdAddr) - 3) * 0x80)) / 0x10)

// Macro for the timeslice in the format required by setTIMER (aka nubers of ticks)
#define TIMESLICE_TICKS (TIMESLICE * (*((cpu_t*) TIMESCALEADDR)))

#endif /* UTILS_H */
