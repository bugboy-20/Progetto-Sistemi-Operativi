#include <pandos_const.h>
#include <list.h>
#include "pcb.h"

/* free pcb list */
static struct list_head pcbFree_h;
/* pcb bucket */
static pcb_t pcbFree_table[MAXPROC];

/* initialize every pbc's field to 0 or NULL */
static void definePcb(pcb_t *p);
/* check if the list_head points to NULL */
static bool isUndefined(struct list_head *);

void initPcbs()
{
    INIT_LIST_HEAD(&pcbFree_h);

    // Initialize every process as free
    for (int i = 0; i < MAXPROC; i++)
        list_add(&pcbFree_table[i].p_list, &pcbFree_h);
}

void freePcb(pcb_t *p)
{
    if (p == NULL)
        return;

    // Remove the pcb from active process' list
    if (!isUndefined(&p->p_list))
        list_del_init(&p->p_list);
    list_add(&p->p_list, &pcbFree_h);
}

pcb_t *allocPcb()
{
    if (list_empty(&pcbFree_h))
        return NULL;

    // Get first free process
    pcb_t *toBeAllocated = list_first_entry(&pcbFree_h, pcb_t, p_list);

    // remove toBeAllocated from free process list
    list_del(&toBeAllocated->p_list);
    definePcb(toBeAllocated);

    return toBeAllocated;
}

void mkEmptyProcQ(struct list_head *head)
{
    INIT_LIST_HEAD(head);
}

int emptyProcQ(struct list_head *head)
{
    return list_empty(head);
}

void insertProcQ(struct list_head *head, pcb_t *p)
{
    // If empty initialize list
    if (emptyProcQ(head))
        INIT_LIST_HEAD(head);

    list_add_tail(&p->p_list, head);
}

pcb_t *headProcQ(struct list_head *head)
{
    if (emptyProcQ(head))
        return NULL;
    
    return list_first_entry(head, pcb_t, p_list);
}

pcb_t *removeProcQ(struct list_head *head)
{
    if (emptyProcQ(head))
        return NULL;
    
    pcb_t *tmp = headProcQ(head);
    list_del_init(&tmp->p_list);

    return tmp;
}

pcb_t *outProcQ(struct list_head *head, pcb_t *p)
{
    if (emptyProcQ(head))
        return NULL;

    struct pcb_t *tmp;

    // if p found, return it, otherwise return NULL
    list_for_each_entry(tmp, head, p_list) // Iterate for each PCB in process queue
    {
        if (tmp == p)
        {
            list_del_init(&tmp->p_list);
            return p;
        }
    }

    return NULL;
}

int emptyChild(pcb_t *p)
{
    if (p == NULL)
        return TRUE;
    return list_empty(&p->p_child);
}

void insertChild(pcb_t *prnt, pcb_t *p)
{
    if (prnt == NULL || p == NULL)
        return;

    // If p is in another tree, remove the process from there
    if (p->p_parent != NULL)
        outChild(p);

    p->p_parent = prnt;
    list_add_tail(&p->p_sib, &prnt->p_child);
}

pcb_t *removeChild(pcb_t *p)
{
    if (emptyChild(p))
        return NULL;

    // Get first sibling from p_sib list from list pointed by p_child of p, then remove it
    pcb_t *tmp = list_first_entry((&p->p_child), pcb_t, p_sib);
    outChild(tmp);

    return tmp;
}

pcb_t *outChild(pcb_t *p)
{
    if (p == NULL || p->p_parent == NULL)
        return NULL;

    // Remove p from siblilng list and remove the parent
    list_del_init(&p->p_sib);
    p->p_parent = NULL;

    return p;
}

static void definePcb(pcb_t *p)
{
    // init pcb
    INIT_LIST_HEAD(&p->p_list);

    p->p_parent = NULL;
    INIT_LIST_HEAD(&p->p_child);
    INIT_LIST_HEAD(&p->p_sib);

    // init state_t
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

    for (int i = 0; i < NS_TYPE_MAX; i++)
        p->namespaces[i] = NULL;
}

static bool isUndefined(struct list_head *l)
{
    // It never happens that only next or prev is NULL
    return (l->next == NULL && l->prev == NULL);
}