#include "../include/ash.h"
#include "hashtable.h"
#include "list.h"
#include <types.h>
#include <pandos_const.h>
#include <pandos_types.h>

static semd_t semd_table[MAXPROC];
static LIST_HEAD(semdFree_h);
static DEFINE_HASHTABLE(semd_h, 5);

int insertBlocked(int *semAdd, pcb_t *p) {
    semd_t *sem;

    hash_for_each_possible(semd_h,sem,s_link,*semAdd) { //assumo che avvenga al più un solo giro essendo la chiave univoca (no?)
        list_add_tail(&p->p_list, &sem->s_procq); //TODO non sono affatto sicuro che devo appenderlo per p_list
        return FALSE;
    }

    //SEM non nella ASH -> Bisogna allocarne uno

    if(list_empty(&semdFree_h))
        return TRUE;

    //rimuovo il semaforo da semdFree_h
    sem = list_first_entry(&semdFree_h,semd_t,s_freelink);
    list_del(&sem->s_freelink);

    //set dei campi ed aggiunta
    sem->s_key=semAdd;
    INIT_LIST_HEAD(&sem->s_procq);
    hash_add(semd_h, &sem->s_link, *semAdd);


    return FALSE;
}

pcb_t *removeBlocked(int *semAdd) {
    semd_t *sem=NULL;
    pcb_t *pcb=NULL;

    hash_for_each_possible(semd_h, sem, s_link, *semAdd) {
        //serve un controllo?
        pcb = list_first_entry(&sem->s_procq, pcb_t, p_list);
        list_del(&pcb->p_list);

        if (list_empty(&sem->s_procq)) {
            hash_del(&sem->s_link);
            list_add(&sem->s_freelink, &semdFree_h);
        }
    }
    return pcb;
}
/**
 X Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato (indicato da p->p_semAdd).
 ? Se il PCB non compare in tale coda, allora restituisce NULL (condizione di errore). Altrimenti, restituisce p.
 X Se la coda dei processi bloccati per il semaforo diventa vuota,
 X rimuove il descrittore corrispondente dalla ASH e lo inserisce nella coda dei descrittori liberi
 */
pcb_t *outBlocked(pcb_t *p) {

    semd_t *sem=NULL;

    hash_for_each_possible(semd_h, sem, s_link, *p->p_semAdd); //forse anche questo è un metodo leggittimo per interrogare l'ASH

    if(sem==NULL)
        return NULL;

    list_del(&p->p_list);
    p->p_semAdd=NULL; //immagino...

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
    semd_t *sem=NULL;

    hash_for_each_possible(semd_h, sem, s_link, *semAdd);

    if(sem==NULL || list_empty(&sem->s_procq))
        return NULL;
    return list_first_entry(&sem->s_procq, pcb_t, p_list);
}
/**
 * Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable.
 * Questo metodo viene invocato una volta sola durante l’inizializzazione della struttura dati.
 */
void initASH() {
    INIT_LIST_HEAD(&semdFree_h);
    for(int i=0; i<MAXPROC; i++)
        list_add(&semd_table[i].s_freelink, &semdFree_h);
}
