#include <pandos_types.h>
// istruzioni per la terminazione sono al capitolo 3.9 del manuale
void terminate_recursively(pcb_t *proc);
pcb_t *get_proc_by_pid(int pid);
void scheduler();
// TODO: da togliere e sotituire con una variabile globale
pcb_t *processo_attuale;
struct list_head *readyQ;