#include "ash.h"
#include <hashtable.h>

static semd_t semd_table[MAXPROC];
static LIST_HEAD(semdFree_h);
static DEFINE_HASHTABLE(semd_h, const_ilog2(MAXPROC));

/**
 * getSemd - search a semd inside semd_h
 * @semAdd: semaphore's address
 *
 * @return: pointer to semd if found, NULL o.w.
 */
static inline semd_t *getSemd(int *semAdd) {
    semd_t *sem;
    hash_for_each_possible(semd_h, sem, s_link, hash32_ptr(semAdd)) {
        if(sem->s_key==semAdd)
            return sem;
    }
    return NULL;
}

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

pcb_t *removeBlocked(int *semAdd) {
    return outBlocked(headBlocked(semAdd));
}

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

pcb_t *headBlocked(int *semAdd) {
    semd_t *sem=getSemd(semAdd);

    if(sem==NULL || list_empty(&sem->s_procq))
        return NULL;

    return list_first_entry(&sem->s_procq, pcb_t, p_list);

    return NULL;
}

void initASH() {
    for(int i=0; i<MAXPROC; i++)
        list_add(&semd_table[i].s_freelink, &semdFree_h);
}
