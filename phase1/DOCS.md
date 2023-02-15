# Documentazione progetto SO fase 1

## Controlli ridondanti o non richiesti
Abbiamo deciso di fare un controllo solo in alcuni casi nonostante non fosse necessario con un utilizzo corretto dei metodi.

### `freePcb`
Eliminiamo il PCB da una precedente lista facendo attenzione che i puntatori prev e next di p_list non siano NULL.

### `insertProcQ`
Se la lista è vuota la inizializziamo prima di inserirci un nuovo elemento.

### `insertChild`
Se p appartiene già all'albero allora viene spostato su quello nuovo.

### `outBlocked`
In caso p sia nullo viene ritornato nullo.

### `headBlocked`
semAdd può essere sia un valore nullo che invalido senza causare comportamenti inaspettati.

## Utilizzo di `list_del_init`
Per eliminare un elemento da una lista utilizziamo sempre `list_del_init` perchè evita di lasciare dei puntatori a NULL. Questo potrebbe generare problemi in caso di utilizzo successivo di `list_empty` e operazioni con puntatori.

L'unica eccezione è in `allocPcb` dove sarebbe inutile perchè utilizziamo `definePcb` subito dopo.

## Implementazione dei namespace
I namespace non sono stati implementati attraverso array, ma con una matrice di dimensione $\texttt{NS\_TYPE\_MAX} \times \texttt{MAXPROC}$.
Questa implementazione permette di generalizzare le funzioni dei namespace, rendendole utilizzabili con un qualsiasi numero di tipi.

Anche le liste di namespace liberi e attivi sono state implementate tramite array, 
permettendo così di accedere direttamente alla lista del tipo specifico, senza avere liste diversi per ogni tipo di namespace
