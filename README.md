# Progetto-Sistemi-Operativi

## Divisione Parti - Fase 1

- Piske, Simo => PCB
- Diego => semafori
- Michele => namespace

In particolare:

- Diego [14..18]
- Simo [4..9]
- Piske [1..3] ∪ [10..13]
- Michele [19..23]



## Divisione parti - Fase 2

1. Inizializzazione del sistema => Diego, Simone
2. Scheduling => *ci pensiamo quando abbiamo fatto la parte 1*
3. Gestione delle eccezioni => Piske, Michele

Se servono delle funzioni non implementate, inserire la signature in un file *TODO.h*

### cose da implementare subito

main.c -> contiene il main(), quindi inizializza e fa partire il *p2test*

syscall.h -> contiene gli header delle syscall

### idea file structure

```
+
|- include
	|- syscall.h
    |- exception.h
    |- scheduler.h
|- src
	|- main.c
	|- exception.c
	|- scheduler.c
	|- syscall.c
|- Makefile
```


## Link Utili

- [Libro su umps3](http://cs.unibo.it/~renzo/doc/umps3/uMPS3princOfOperations.pdf)
- [capitolo 2 del libro](http://cs.unibo.it/~renzo/so/panda+/panda+.pdf)
- [pdf del progetto p1](http://cs.unibo.it/~renzo/so/panda+/panda+phase1.pdf)
- [pdf del progetto p2](http://cs.unibo.it/~renzo/so/panda+/panda+phase2.pdf)
- [manuale](http://cs.unibo.it/~renzo/doc/umps3/pandos.pdf)
