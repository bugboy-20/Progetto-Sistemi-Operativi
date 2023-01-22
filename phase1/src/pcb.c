#include <pandos_const.h>
#include <list.h>
#include "../include/pcb.h"

static struct list_head pcbFree_h;
static pcb_t pcbFree_table[MAXPROC];
static void definePcb(pcb_PTR p);

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

    definePcb(toBeAllocated);

    return toBeAllocated;
}

void mkEmptyProcQ(struct list_head *head) {}
int emptyProcQ(struct list_head *head) { return 420; }
void insertProcQ(struct list_head *head, pcb_t *p) {}
pcb_t *headProcQ(struct list_head *head) { return NULL; }
pcb_t *removeProcQ(struct list_head *head) { return NULL; }
pcb_t *outProcQ(struct list_head *head, pcb_t *p) { return NULL; }

int emptyChild(pcb_t *p)
{
    if (p == NULL)
        return TRUE;
    return list_empty(&p->p_child);
}

void insertChild(pcb_t *prnt, pcb_t *p)
{
    if (prnt == NULL || p == NULL) /* TODO: devo controllare se ha già un padre?*/
        return;

    p->p_parent = prnt;
    list_add(&p->p_sib, &prnt->p_child); /* TODO: cosa succede se p ha già dei siblings*/
}

pcb_t *removeChild(pcb_t *p)
{
    if (emptyChild(p))
        return NULL;

    pcb_PTR firstChild = container_of(&p->p_child.next, pcb_t, p_sib);
    return outChild(firstChild);
}

pcb_t *outChild(pcb_t *p)
{
    if (p == NULL || p->p_parent == NULL)
        return NULL;

    list_del(&p->p_sib); /*remove p from the sibling list*/
    p->p_sib.next = p->p_sib.prev = NULL;
    p->p_parent = NULL;
    return p;
}

static void definePcb(pcb_PTR p)
{
    /* init pcb */
    INIT_LIST_HEAD(&p->p_list);

    p->p_parent = NULL;
    INIT_LIST_HEAD(&p->p_child);
    INIT_LIST_HEAD(&p->p_sib);

    /* init state_t */
    p->p_s.entry_hi = 0;
    p->p_s.cause = 0;
    p->p_s.status = 0;
    p->p_s.pc_epc = 0;
    p->p_s.hi = 0;
    p->p_s.lo = 0;
    for (int i = 0; i < STATE_GPR_LEN; i++)
        p->p_s.gpr[i] = 0;

    p->p_time = 0;

    p->p_semAdd = NULL;

    // TODO: capire se è giusto allocare i ns così
    for (int i = 0; i < NS_TYPE_MAX; i++)
        p->namespaces[i] = NULL;
}