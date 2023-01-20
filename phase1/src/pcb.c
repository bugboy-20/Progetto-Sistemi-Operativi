#include <pandos_const.h>
#include <list.h>
#include "../include/pcb.h"

static struct list_head pcbFree_h;
static pcb_t pcbFree_table[MAXPROC];

void initPcbs()
{
    INIT_LIST_HEAD(&pcbFree_h);

    for (int i = 0; i < MAXPROC; i++)
    {
        list_add(&pcbFree_table[i].p_list, &pcbFree_h);
    }
}

void freePcb(pcb_t *p)
{
    if (p == NULL) // TODO: controllare se p non è già libero?
        return;

    list_add(&p->p_list, &pcbFree_h);
}

pcb_t *allocPcb()
{
    if (list_empty(&pcbFree_h))
        return NULL;

    pcb_PTR toBeAllocated = container_of(pcbFree_h.next, pcb_t, p_list);
    list_del(pcbFree_h.next);

    if (toBeAllocated == NULL)
        return NULL;

    /* init pcb */
    INIT_LIST_HEAD(&toBeAllocated->p_list);

    toBeAllocated->p_parent = NULL;
    INIT_LIST_HEAD(&toBeAllocated->p_child);
    INIT_LIST_HEAD(&toBeAllocated->p_sib);

    /* init state_t */
    toBeAllocated->p_s.entry_hi = 0;
    toBeAllocated->p_s.cause = 0;
    toBeAllocated->p_s.status = 0;
    toBeAllocated->p_s.pc_epc = 0;
    toBeAllocated->p_s.hi = 0;
    toBeAllocated->p_s.lo = 0;
    for (int i = 0; i < STATE_GPR_LEN; i++)
        toBeAllocated->p_s.gpr[i] = 0;

    toBeAllocated->p_time = 0;

    toBeAllocated->p_semAdd = NULL;

    // TODO: capire se è giusto allocare i ns così
    for (int i = 0; i < NS_TYPE_MAX; i++)
        toBeAllocated->namespaces[i] = NULL;

    return toBeAllocated;
}