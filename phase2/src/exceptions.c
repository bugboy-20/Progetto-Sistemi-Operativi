// Parte di Pische e Michele
#include <exceptions.h>
#include <pandos_const.h>
#include "../include/syscall.h"

// costanti temporanee
// finchè non abbiamo i file del prof
#define GETPROCESSID 9
#define GET_CHILDREN 10

int SYSCALL(int a0, void *a1, void *a2, void *a3)
{
    switch (a0)
    {
    // Pische
    case CREATEPROCESS:
        return create_process((state_t *)a1, (struct support_t *)a2, (nsd_t *)a3);
    case TERMPROCESS:
        terminate_process(*(int *)a1);
        break;
    case PASSEREN:
        passeren((int *)a1);
        break;
    case VERHOGEN:
        verhogen((int *)a1);
        break;
    case IOWAIT:
        return do_io((int *)a1, (int *)a2);
    // Michele
    case GETTIME:
        return get_cpu_time();
    case CLOCKWAIT:
        return wait_for_clock();
    case GETSUPPORTPTR:
        return get_support_data();
    case GETPROCESSID:
        return get_process_id(*(int *)a1);
    case GET_CHILDREN:
        return get_children((int *)a1, *(int *)a2);
    default:
        return -1;
    }
    // ho assunto che il valore di ritorno per le syscall void sia 0
    return 0;
}
