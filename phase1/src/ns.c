
#ifndef NS_H
#define NS_H

#include <umps/const.h>
#include <pandos_const.h>
#include <pandos_types.h>


/*
type_nsd[MAX_PROC]: diversi array
(uno per tipo di namespace! Per
fase1 basta un singolo namespace:
PID) di NSD con dimensione
massima di MAX_PROC.
• type_nsFree_h: Lista dei NSD di tipo
type liberi o inutilizzati.
• type_nsList_h: Lista dei
namespace di tipo type attivi.
*/

static nsd_t nsd[NS_TYPE_MAX][MAXPROC];
struct list_head nsFree_h[NS_TYPE_MAX];
struct list_head nsList_h[NS_TYPE_MAX];

void initNamespaces() {
    for(int j=0; j<NS_TYPE_MAX; j++)
    {
        INIT_LIST_HEAD(&nsFree_h[j]);
        INIT_LIST_HEAD(&nsList_h[j]);
        for(int i=0; i<MAXPROC;i++)//scorro gli array di tipo j
        {
            nsd[j][i].n_type = j;
            list_add(&nsd[NS_PID][i].n_link, &nsFree_h[j]);//aggiungo i namespace alla lista di quelli liberi
        }
    }
}

nsd_t *getNamespace(pcb_t *p, int type) {
    return p->namespaces[type]; //se namespace[type] è null ritorna null, altrimenti ritorna il namespace
}

int addNamespace(pcb_t *p, nsd_t *ns) {
    //si assume un namespace già allocato
    if(p !=NULL && ns != NULL){
        p->namespaces[ns->n_type] = ns;//assegnamento al processo padre
        pcb_t* i;
        list_for_each_entry(i, &p->p_child, p_sib) //scorro i figli per assegnare il namespace
        {
            i->namespaces[ns->n_type] = ns;
        }
        return TRUE;
    }
    return FALSE; // errore in caso di processo o namespace nullo
}

nsd_t *allocNamespace(int type) {
    if(!list_empty(&nsFree_h[type]))
    {
        nsd_t *ns = list_first_entry(&nsFree_h[type], nsd_t, n_link);
        list_move(&ns->n_link, &nsList_h[type]); //sposta ns nella lista dei namespace liberi del suo tipo
        return ns;
    }
    return NULL; //ritorna null se non ci sono namespace liberi
}

void freeNamespace(nsd_t *ns) {
    if(ns != NULL)
        list_move(&ns->n_link, &nsFree_h[ns->n_type]); //sposta ns nella lista dei namespace liberi del suo tipo
}

#endif // NS_H
