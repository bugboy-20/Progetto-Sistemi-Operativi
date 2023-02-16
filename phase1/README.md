Per la compilazione eseguire il seguente comando dalla cartella principale

```
$ make
```

Crea i file `kernel.core.umps` e `kernel.stab.umps` per l'esecuzione su `umps3`

I file oggetto sono dentro la cartella `build/`, dentro `lib/` ci sono i file header forniti, dentro `include/` ci sono i file header fatti da noi.

Dentro a `machine_conf` c'è il file con la configurazione della macchina, e dentro `src/` i file sorgente in C.

Nel file di test `p1test.c` abbiamo cambiato gli include per la compatibilità sui nostri sistemi, è funzionante sulle macchine di laboratorio ugualmente.
