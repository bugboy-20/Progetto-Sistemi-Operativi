#include "../include/ash.h"
#include "hash.h"
#include "hashtable.h"
#include "list.h"
#include <types.h>
#include <pandos_const.h>
#include <pandos_types.h>

static semd_t semd_table[MAXPROC];
static LIST_HEAD(semdFree_h);
static DEFINE_HASHTABLE(semd_h, 5);


static inline semd_t *getSemd(int *semAdd) {
    semd_t *sem;
    hash_for_each_possible(semd_h, sem, s_link, hash32_ptr(semAdd)) {
        if(sem->s_key==semAdd)
            return sem;
    }
    return NULL;
}

/* Viene inserito il PCB puntato da p nella coda dei processi bloccati associata al SEMD con chiave semAdd.
 * Se il semaforo corrispondente non è presente nella ASH, alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e lo inserisce nella ASH, settando I campi in maniera opportuna (i.e.key e s_procQ).
 * Se non è possibile allocare un nuovo SEMD perché la lista di quelli liberi è vuota, restituisce TRUE.
 * In tutti gli altri casi, restituisce FALSE.
*/
int insertBlocked(int *semAdd, pcb_t *p) {
    semd_t *sem=getSemd(semAdd);

    if(sem==NULL) { //SEM non nella ASH -> Bisogna allocarne uno

        if(list_empty(&semdFree_h))
            return TRUE;



        //rimuovo il semaforo da semdFree_h
        sem = list_first_entry(&semdFree_h,semd_t,s_freelink);
        list_del(&sem->s_freelink);

        //set dei campi ed aggiunta
        sem->s_key=semAdd;
        INIT_LIST_HEAD(&sem->s_procq);
        hash_add(semd_h, &sem->s_link, hash32_ptr(semAdd));
    }


    p->p_semAdd=semAdd;
    list_add_tail(&p->p_list, &sem->s_procq);


    return FALSE;
}

/*
 * Ritorna il primo PCB dalla coda dei processi bloccati (s_procq) associata al SEMD dellaASH con chiave semAdd.
 X Se tale descrittore non esiste nella ASH, restituisce NULL.
 * Altrimenti, restituisce l’elemento rimosso.
 * Se la coda dei processi bloccati per il semaforo diventa vuota,
 *      rimuove il descrittore corrispondente dalla ASH
 *      e lo inserisce nella coda dei descrittori liberi (semdFree_h).
*/
pcb_t *removeBlocked(int *semAdd) {
    return outBlocked(headBlocked(semAdd));
}
/**
 X Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato (indicato da p->p_semAdd).
 X Se il PCB non compare in tale coda, allora restituisce NULL (condizione di errore). Altrimenti, restituisce p.
 X Se la coda dei processi bloccati per il semaforo diventa vuota,
 X rimuove il descrittore corrispondente dalla ASH e lo inserisce nella coda dei descrittori liberi
 */
pcb_t *outBlocked(pcb_t *p) {

    semd_t *sem;

    if(p==NULL || (sem=getSemd(p->p_semAdd))==NULL)
        return NULL;

    list_del(&p->p_list);
    p->p_semAdd=NULL;

    if(list_empty(&sem->s_procq)) {
        hlist_del(&sem->s_link);
        list_add(&sem->s_freelink, &semdFree_h);
    }

    return p;
}
/**
 X Restituisce (senza rimuovere) il puntatore al PCB che si trova in testa
 X alla coda dei processi associata al SEMD con chiave semAdd.
 X Ritorna NULL se il SEMD non compare nella ASH oppure se compare ma la sua coda dei processi è vuota.
 */
pcb_t *headBlocked(int *semAdd) {
    semd_t *sem=getSemd(semAdd);

    if(sem==NULL || list_empty(&sem->s_procq))
        return NULL;

    return list_first_entry(&sem->s_procq, pcb_t, p_list);

    return NULL;
}
/**
 X Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable.
 x Questo metodo viene invocato una volta sola durante l’inizializzazione della struttura dati.
 */
void initASH() {
    //INIT_LIST_HEAD(&semdFree_h);
    for(int i=0; i<MAXPROC; i++)
        list_add(&semd_table[i].s_freelink, &semdFree_h);
}
