
#ifndef NS_H
#define NS_H

#include <umps/const.h>
#include <pandos_const.h>
#include <pandos_types.h>

/**
 * Inizializza tutte le liste dei namespace liberi.
 * Questo metodo viene invocato una volta sola durante l’inizializzazione della struttura dati.
 */
void initNamespaces() {}
/**
 * Ritorna il namespace di tipo type associato al processo p (o NULL).
 */
nsd_t *getNamespace(pcb_t *p, int type) {
    return NULL;
}
/**
 * Associa al processo p e a tutti i suoi figli il namespace ns.
 * Ritorna FALSE in caso di errore, TRUE altrimenti.
 */
int addNamespace(pcb_t *p, nsd_t *ns) {
    return FALSE;
}
/**
 * Alloca un namespace di tipo type dalla lista corretta.
 */
nsd_t *allocNamespace(int type) {
    return NULL;
}
/**
 * Libera il namespace ns ri-inserendolo nella lista di namespace corretta.
 */
void freeNamespace(nsd_t *ns) {}

#endif // NS_H
