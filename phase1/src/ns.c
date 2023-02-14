#ifndef NS_H
#define NS_H

#include <pandos_types.h>
#include "../include/ns.h"

/* namespace matrix, an array for every namespace type(NS_TYPE_MAX) with MAXPROC dimension */
static nsd_t nsd[NS_TYPE_MAX][MAXPROC];
/*nsFree_h: an array of lists of free namespaces for every type*/
static struct list_head nsFree_h[NS_TYPE_MAX];
/*nsFree_h: an array of lists of active namespaces for every type*/
static struct list_head nsList_h[NS_TYPE_MAX];

void initNamespaces() {
    //for every namespace type initialize free and active namespaces lists
    for(int j=0; j<NS_TYPE_MAX; j++)
    {
        INIT_LIST_HEAD(&nsFree_h[j]);
        INIT_LIST_HEAD(&nsList_h[j]);
        //initialize the j type namespace's array and add the to the free namespaces list
        for(int i=0; i<MAXPROC;i++)
        {
            nsd[j][i].n_type = j;
            list_add(&nsd[NS_PID][i].n_link, &nsFree_h[j]);
        }
    }
}

nsd_t *getNamespace(pcb_t *p, int type) {
    //returns namespaces[type] because every pcb has only one namespace for every type
    return p->namespaces[type];
}

int addNamespace(pcb_t *p, nsd_t *ns) {
    //assumes already allocated namespace
    if(p !=NULL && ns != NULL){
        //add namespace ns to the father
        p->namespaces[ns->n_type] = ns;
        pcb_t* i;
        //cycles the childs to add the namespace ns
        list_for_each_entry(i, &p->p_child, p_sib) 
        {
            i->namespaces[ns->n_type] = ns;
        }
        return TRUE;
    }
    // error if null process or namespace
    return FALSE; 
}

nsd_t *allocNamespace(int type) {
    if(!list_empty(&nsFree_h[type]))
    {
        //move the first free namespace to the active namespaces list of its type
        nsd_t *ns = list_first_entry(&nsFree_h[type], nsd_t, n_link);
        list_move(&ns->n_link, &nsList_h[type]);
        return ns;
    }
    //returns null if there are no free namespaces
    return NULL;
}

void freeNamespace(nsd_t *ns) {
    if(ns != NULL)
        //move ns to the free namespaces list of its type
        list_move(&ns->n_link, &nsFree_h[ns->n_type]);
}

#endif // NS_H
