# Documentazione progetto SO fase 2

## TODO

- macro per semafori
- ~~come abbiamo salvato i vari semafori~~
- ~~WAIT che proseguiva, come abbiamo risolto~~
- ~~DOIO~~
  - ~~Come trovare l'indirizzo giusto~~
  - ~~Come restituire il valore corretto (array values)~~
- ~~Gestione di P e di V diverse dalle system call passeren e verhogen~~
- ~~pid sono i propri indirizzi~~

## Scelte implementative

Sono qui sotto elencate cosa abbiamo fatto nei casi in cui la scelta è stata lasciata libera

### Semafori

Abbiamo implementato semafori binari, quindi gli unici valori accettati sono 0 e 1. Una P sul valore 0 è bloccante, una V sul valore 1 è bloccante.

Le operazioni di P e V sono implementate in un file a parte. Abbiamo fatto questa scelta perché in questo modo è possibile avere più flessibilità di utilizzo e meno codice ripetuto. 
Infatti vengono richiamate nelle systemcall passeren e verhogen, ma anche in altri contesti.
Le due funzioni restituiscono un valore booleano: _true_ nel caso in cui la chiamata sia bloccante, _false_ altrimenti.


### Gestione dei semafori del nucleo

I semafori dei device gestiti dal nucleo li abbiamo implementati tramite un array di 48 elementi, questo perchè ci sono 5 interrupt line utilizzate dai device e per ognuna 8 device, più il terminale che ha 16 device (8 trasmissione, 8 ricezione), in totale `4*8+16 = 48`. 
Per accedere comodamente a questo array esiste una macro `dev_sem_addr(type, n)` che dato il tipo e il numero di device restituisce l'indirizzo del semaforo corrispondente.

Esiste anche una variabile globale `int pseudoclock_semaphore` che gestisce il semaforo dell'interval timer.

### Gestione delle eccezioni

La funzione che gestisce le eccezioni è `exception_handler()` e si occupa di richiamare la procedura corretta come da specifica. Può richiamare `syscall_handler()` che si occupa solo delle syscall. Oppure può richiamare `interrupt_handler()` che si occupa solo degli interrupt.

### Terminazione delle syscall

Subito prima della terminazione di ogni syscall viene richiamata una funzione `syscall_end(bool terminated, bool blocked)` che si occupa di generalizzare la corretta procedura per la terminazione. I casi da gestire sono tre:

1.  il processo corrente termina
2.  il processo corrente non termina e passa dallo stato _running_ a _waiting_
3.  il processo corrente non termina e continua la sua esecuzione

I due parametri booleani servono per identificare in quale stato si trova, **terminated** è _true_ se siamo nel primo caso, se è _false_ si ricorre a **blocked**, infatti se è _true_ siamo nel secondo caso, altrimenti si ricade nel terzo caso.

### Gestione I/O

Per quanto riguarda la systemcall 5 `do_io(int *cmdAddr, int *cmdValues)`, per prima cosa estrapoliamo dall'indirizzo il tipo e il numero del device. 
Viene fatto tramite le macro `getTypeDevice(addr)` e `getNumDevice(addr)`.
Visto che è necessario scrivere l'esito dell'operazione al posto del comando, salviamo l'indirizzo in una variabile globale `value_bak`.

### Terminazione dei processi

La terminazione di un processo avviene tramite una funzione ricorsiva sui figli di tale processo.

### Process Id

Abbiamo deciso di utilizzare l'indirizzo del pcb come PID del processo. 
Questo perché è più comodo rispetto a gestire un indice incrementale, inoltre semplifica anche la funzione che trova il pcb dato il PID.

### Funzioni per gestire i device

I device e i loro indirizzi sono stati gestiti attraverso piccole funzioni e macro, ciò semplifica la lettura ed evita di ripetere blocchi di codice.
All'avvento di un interrupt il device viene dedotto scorrendo iterativamente bit a bit la bit map degli interrupts.


### Interval timer sempre caricato con il tempo corretto, mai con il tempo massimo (TODO non ho idea)

Il comando LDIT viene sempre eseguito usando 100ms, questo perché allo scattare del timer interrupt l'interval timer viene reimpostato a `0xFF FF FF FF`, quindi prima che si ripresenti abbiamo a disposizione $5·10^9\sim$ cicli di clock (che con la CPU impostata a 1MHz corrisponde a 1h30~), più che sufficienti per eseguire le operazioni richieste senza doverlo riportare al massimo


## Problemi riscontrati

### Problema con lo stato WAIT

A volte accadeva che andasse in WAIT senza gestire correttamente gli interrupt bloccando di fatto l'esecuzione dell'intero sistema.
Per risolvere abbiamo disabilitato gli interrupt sulla linea 1 e 2 (interval timer e PLT).
