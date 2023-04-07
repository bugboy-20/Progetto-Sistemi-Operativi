// Parte di Diego

#include "../include/ash.h"

int main() {
    initPcbs();
    initASH();
    initNamespaces();
    
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
}
