#ifndef SYSCALL_H
#define SYSCALL_H

#include <pandos_types.h>
#include <types.h>

// SYS1
void create_process(state_t *, struct support_t *, nsd_t *);

// SYS2
void terminate_process(int pid);

// SYS3
void passeren(int *semAddr);

// SYS4
void verhogen(int *semAddr);

// SYS5
void do_io(int *cmdAddr, int *cmdValues);

// SYS6
void get_cpu_time();

// SYS7
void wait_for_clock();

// SYS8
void get_support_data();

// SYS9
void get_process_id(bool parent);

// SYS10
void get_children(int *children, int size);

// SYSTEM CALL 1-5 -> PISKE
//             6-10 -> MICHELE

#endif /* SYSCALL_h */