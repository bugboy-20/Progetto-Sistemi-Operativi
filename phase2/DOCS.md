# Documentazione progetto SO fase 2

## TODO
- syscall_end
- macro per semafori
- come abbiamo salvato i vari semafori
- WAIT che proseguiva, come abbiamo risolto
- DOIO
  - Come trovare l'indirizzo giusto
  - Come restituire il valore corretto (array values)
- Interval timer sempre caricato con il tempo corretto, mai con il tempo massimo
- Gestione di P e di V diverse dalle system call passeren e verhogen
- pid sono i propri indirizzi

## Scelte implementative

Sono qui sotto elencate cosa abbiamo fatto nei casi in cui la scelta è stata lasciata libera

### Semafori

Abbiamo implementato semafori binari, quindi gli unici valori accettati sono 0 e 1. Una P sul valore 0 è bloccante, una V sul valore 1 è bloccante.

Le operazioni di P e V sono implementate (nelle syscall correlate | in un file a parte)

### Gestione delle eccezioni

La funzione che gestisce le eccezioni è `exception_handler()` e si occupa di richiamare la procedura corretta come da specifica. Può richiamare `syscall_handler()` che si occupa solo delle syscall. Oppure può richiamare `interrupt_handler()` che si occupa solo degli interrupt.

### Terminazione delle syscall

Subito prima della terminazione di ogni syscall viene richiamata una funzione `syscall_end(bool terminated, bool blocked)` che si occupa di eseguire la corretta procedure per la terminazione. I casi da gestire sono tre:

1.  il processo corrente termina
2.  il processo corrente non termina e continua la sua esecuzione
3.  il processo corrente non termina e passa dallo stato _running_ a _waiting_

I due parametri booleani servono per identificare in quale stato si trova, **terminated** è _true_ se siamo nel primo caso, se è _false_ si ricorre a **blocked**, infatti se è _false_ siamo nel secondo caso, altrimenti si ricade nel terzo caso.

### Terminazione dei processi

La terminazione di un processo avviene tramite una funzione ricorsiva sui figli di tale processo.

### Funzioni per gestire i device
