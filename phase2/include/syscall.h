#include "pandos_types.h"

// SYS1
int create_process(state_t *, struct support_t *, nsd_t *);

// SYS2
void terminate_process(int pid);

// SYS3
void passeren(int *semAddr);

// SYS4
void verhogen(int *semAddr);

// SYS5
int do_io(int *cmdAddr, int *cmdValues);

// SYS6
int get_cpu_time();

// SYS7
int wait_for_clock();

// SYS8
struct support_t *get_support_data();

// SYS9
int get_process_id(int parent);

// SYS10
int get_children(int *children, int size);

// SYSTEM CALL 1-5 -> PISKE
//             6-10 -> MICHELE