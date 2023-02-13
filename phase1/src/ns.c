
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


static nsd_t nsd[NS_TYPE_MAX][MAXPROC];
struct list_head nsFree_h[NS_TYPE_MAX];
struct list_head nsList_h[NS_TYPE_MAX];

/**
 * Inizializza tutte le liste dei namespace liberi.
 * Questo metodo viene invocato una volta sola durante l’inizializzazione della struttura dati.
 */
void initNamespaces() {
    for(int j=0; j<NS_TYPE_MAX; j++)
    {
        INIT_LIST_HEAD(&nsFree_h[j]);
        INIT_LIST_HEAD(&nsList_h[j]);
    for(int i=0; i<MAXPROC;i++)//scorro gli array di tipo PID
    {
        nsd[j][i].n_type = j;
        list_add(&nsd[NS_PID][i].n_link, &nsFree_h[j]);//aggiungo i namespace alla lista di quelli liberi
    }
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
            if(p->namespaces[i]->n_type == type)
                return p->namespaces[i]; //ritorna il namespace del tipo type
        }    
    }
    return NULL;
}

/**
 * Associa al processo p e a tutti i suoi figli il namespace ns.
 * Ritorna FALSE in caso di errore, TRUE altrimenti.
 */
int addNamespace(pcb_t *p, nsd_t *ns) {
    //si assume un namespace già allocato
    if(p !=NULL && ns != NULL){
        p->namespaces[ns->n_type] = ns;//assegnamento al padre
        pcb_t* i;
        list_for_each_entry(i, &p->p_child, p_sib) //assegnamento ai figli
        {
            i->namespaces[ns->n_type] = ns;
        }
        return TRUE;
    }
    return FALSE; // errore in caso di processo o namespace nullo
}

/**
 * Alloca un namespace di tipo type dalla lista corretta.
 */
nsd_t *allocNamespace(int type) {
    if(!list_empty(&nsFree_h[type]))
    {
        nsd_t *ns = list_first_entry(&nsFree_h[type], nsd_t, n_link);
        list_move(&ns->n_link, &nsList_h[type]);
        return ns;
    }
}


/**
 * Libera il namespace ns ri-inserendolo nella lista di namespace corretta.
 */
void freeNamespace(nsd_t *ns) {
    list_move(&ns->n_link, &PID_nsFree_h);
}

#endif // NS_H
