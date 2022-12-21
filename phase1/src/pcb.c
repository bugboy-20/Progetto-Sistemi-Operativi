#include <pandos_const.h>
#include <list.h>
#include "../include/pcb.h"

static struct list_head pcbFree_h;
static pcb_t pcbFree_table[MAXPROC];

void initPcbs() // NOTA: non testato, probabilmente non funziona
{
    INIT_LIST_HEAD(&pcbFree_h);

    for (int i = 0; i < MAXPROC; i++)
    {
        // TODO: forse il bucket dei pcb deve essere un array di puntatori,
        // e di conseguenza anche la variabile qui sotto
        struct pcb_t new_pcb;
        LIST_HEAD(temp);
        new_pcb.p_list = temp;
        pcbFree_table[i] = new_pcb;

        list_add(&pcbFree_table[i].p_list, &pcbFree_h);
    }
}

void freePcb(pcb_t *p) // NOTA: non testato, probabilmente non funziona
{
    list_add(&p->p_list, &pcbFree_h);
}

pcb_t *allocPcb() // NOTA: non testato, probabilmente non funziona
{
    if (list_empty(&pcbFree_h))
        return NULL;
    else
    {
        pcb_PTR toBeAllocated = container_of(pcbFree_h.next, pcb_t, p_list);
        list_del(pcbFree_h.next);

        // inizializzo i campi
        INIT_LIST_HEAD(&toBeAllocated->p_list);

        toBeAllocated->p_parent = NULL;
        INIT_LIST_HEAD(&toBeAllocated->p_child);
        INIT_LIST_HEAD(&toBeAllocated->p_sib);

        // TODO: capire come inizializzare state_t
        // toBeAllocated->p_s = {0, 0, 0, 0, NULL, 0, 0};
        toBeAllocated->p_time = 0;

        toBeAllocated->p_semAdd = NULL;

        // TODO: capire come allocare i namespace
        // toBeAllocated->namespaces

        return toBeAllocated;
    }
}