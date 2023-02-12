
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
static nsd_t PID_nsd[MAXPROC];
static LIST_HEAD(PID_nsFree_h);
static LIST_HEAD(PID_nsList_h);
/**
 * Inizializza tutte le liste dei namespace liberi.
 * Questo metodo viene invocato una volta sola durante l’inizializzazione della struttura dati.
 */
void initNamespaces() {
    for(int i=0; i<MAXPROC;i++)
    {
        PID_nsd[i].n_type = NS_PID;
        list_add(&PID_nsd[i].n_link, &PID_nsFree_h);
    }
}
/**
 * Ritorna il namespace di tipo type associato al processo p (o NULL).
 */
nsd_t *getNamespace(pcb_t *p, int type) {
    for(int i=0;i<NS_TYPE_MAX; i++)
    {
        if(p->namespaces[i] != NULL)
        {
            if(p->namespaces[i]->n_type == NS_PID) //if(p->namespaces[i]->n_type == type)
                return p->namespaces[i];
        }    
    }
    return NULL;
}
/**
 * Associa al processo p e a tutti i suoi figli il namespace ns.
 * Ritorna FALSE in caso di errore, TRUE altrimenti.
 */
int addNamespace(pcb_t *p, nsd_t *ns) {
    //NAMESPACE GIà ALLOCATO
    if(p !=NULL && ns ! =NULL){
        p->namespaces[ns->n_type] = ns;
        pcb_t* i;
        list_for_each_entry(i, &p->p_child, p_sib)
        {
            i->namespaces[ns->n_type] = ns;
        }
        return TRUE;
    }
    return FALSE; // perché dovrebbe fallire non lo so ancora
}
/**
 * Alloca un namespace di tipo type dalla lista corretta.
 */
nsd_t *allocNamespace(int type) {
    if(type == NS_PID && !list_empty(&PID_nsFree_h))
    {
        nsd_t *ns = list_first_entry(&PID_nsFree_h, nsd_t, n_link);
        list_move(&ns->n_link, &PID_nsList_h);
        return ns;
    }
    return NULL;
}
/**
 * Libera il namespace ns ri-inserendolo nella lista di namespace corretta.
 */
void freeNamespace(nsd_t *ns) {
    list_move(&ns->n_link, &PID_nsFree_h);
}

#endif // NS_H
