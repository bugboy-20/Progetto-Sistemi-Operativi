// Parte di Diego

#include "../include/ash.h"
#include "../include/pcb.h"
#include "../include/ns.h"
#include "list.h"
#include "pandos_const.h"

typedef struct{} sem; //TODO

static int alive_proc;
static int blocked_proc;
static LIST_HEAD(ready_q);
static sem sem_l[MAXPROC];

int main() {
    // Inizializzazione delle seguenti variabili
    /*
    Variabili globali per:
    - Conteggio dei processi vivi
    - Conteggio dei processi bloccati
    - Coda dei processi “ready”
    - Puntatore al processo correntemente attivo
    - Un semaforo (e.g. una variabile int) per ogni
    (sub) dispositivo. Non necessariamente tutti
    questi semafori sono sempre attivi.
    - Strutture dati gia’ gestite in fase 1
    */
    
    // Popolare il pass up vector con getore e stack pointer per eccezioni e TLB-Refill

    // Caricare l'Interval Timer
    alive_proc=0;
    blocked_proc=0;

    initPcbs();
    initASH();
    initNamespaces();

    //TODO quando escono le specifiche si capirà me va fatto
    int* intervaltmr = (int*) INTERVALTMR;
    *intervaltmr=100;

    // INIZIALIZZAZIONE SCHEDULER

    /*
     * Inizializzazione: scheduler
        - Allocare un processo (pcb_t) in kernel mode, con interrupt abilitati, stack pointer a RAMTOP e program counter che punta alla funzione test() (fornita da noi).
        - Inserire questo processo nella Ready Queue.
        - invocare lo scheduler
    */

    /*
     * Pass Up Vector
        Nell’evento di un’eccezione uMPS3 salva lo stato
        del processore in una specifica locazione di
        memoria (0x0FFFF000) e carica PC e SP che
        trova nel Pass Up Vector, una struttura che
        dovete popolare all’indirizzo 0x0FFFF900.
        Il Pass Up Vector distingue tra TLB-Refill e tutte
        le altre eccezioni (per distinguere ulteriormente
        si veda il registro Cause).
        Le eccezioni non possono essere annidate.
    */


    
}
