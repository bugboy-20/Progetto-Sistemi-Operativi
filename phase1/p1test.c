/*********************************P1TEST.C*******************************
 *
 *	Test program for the modules ASH and pcbQueues (phase 1).
 *
 *	Produces progress messages on terminal 0 in addition
 *		to the array ``okbuf[]''
 *		Error messages will also appear on terminal 0 in
 *		addition to the array ``errbuf[]''.
 *
 *		Aborts as soon as an error is detected.
 *
 *      Modified by Michael Goldweber on May 15, 2004
 *      Modified by Davide Berardi on November 23, 2022
 */

#include <pandos_const.h>
#include <pandos_types.h>

#include <umps/libumps.h>
#include "include/pcb.h"
#include "include/ash.h"
#include "include/ns.h"


#define MAXPROC 20
#define MAXSEM  MAXPROC
#define MAXNS   MAXPROC

char   okbuf[2048]; /* sequence of progress messages */
char   errbuf[128]; /* contains reason for failing */
char   msgbuf[128]; /* nonrecoverable error message before shut down */
int    sem[MAXSEM];
int    onesem;
pcb_t *procp[MAXPROC], *p, *q, *firstproc, *lastproc, *midproc;
nsd_t *pid_ns, *pid_ns2;
char  *mp = okbuf;


#define TRANSMITTED 5
#define ACK         1
#define PRINTCHR    2
#define CHAROFFSET  8
#define STATUSMASK  0xFF
#define TERM0ADDR   0x10000254

typedef unsigned int devreg;

/* This function returns the terminal transmitter status value given its address */
devreg termstat(memaddr *stataddr) {
    return ((*stataddr) & STATUSMASK);
}

/* This function prints a string on specified terminal and returns TRUE if
 * print was successful, FALSE if not   */
unsigned int termprint(char *str, unsigned int term) {
    memaddr     *statusp;
    memaddr     *commandp;
    devreg       stat;
    devreg       cmd;
    unsigned int error = FALSE;

    if (term < DEVPERINT) {
        /* terminal is correct */
        /* compute device register field addresses */
        statusp  = (devreg *)(TERM0ADDR + (term * DEVREGSIZE) + (TRANSTATUS * DEVREGLEN));
        commandp = (devreg *)(TERM0ADDR + (term * DEVREGSIZE) + (TRANCOMMAND * DEVREGLEN));

        /* test device status */
        stat = termstat(statusp);
        if (stat == READY || stat == TRANSMITTED) {
            /* device is available */

            /* print cycle */
            while (*str != EOS && !error) {
                cmd       = (*str << CHAROFFSET) | PRINTCHR;
                *commandp = cmd;

                /* busy waiting */
                stat = termstat(statusp);
                while (stat == BUSY)
                    stat = termstat(statusp);

                /* end of wait */
                if (stat != TRANSMITTED)
                    error = TRUE;
                else
                    /* move to next char */
                    str++;
            }
        } else
            /* device is not available */
            error = TRUE;
    } else
        /* wrong terminal device number */
        error = TRUE;

    return (!error);
}


/* This function placess the specified character string in okbuf and
 *	causes the string to be written out to terminal0 */
void addokbuf(char *strp) {
    char *tstrp = strp;
    while ((*mp++ = *strp++) != '\0')
        ;
    mp--;
    termprint(tstrp, 0);
}


/* This function placess the specified character string in errbuf and
 *	causes the string to be written out to terminal0.  After this is done
 *	the system shuts down with a panic message */
void adderrbuf(char *strp) {
    char *ep    = errbuf;
    char *tstrp = strp;

    while ((*ep++ = *strp++) != '\0')
        ;

    termprint(tstrp, 0);

    PANIC();
}



int main(void) {
    int i;

    initPcbs();
    addokbuf("Initialized process control blocks   \n");

    /* Check allocProc */
    for (i = 0; i < MAXPROC; i++) {
        if ((procp[i] = allocPcb()) == NULL)
            adderrbuf("allocPcb: unexpected NULL   ");
    }
    if (allocPcb() != NULL) {
        adderrbuf("allocPcb: allocated more than MAXPROC entries   ");
    }
    addokbuf("allocPcb ok   \n");

    /* return the last 10 entries back to free list */
    for (i = 10; i < MAXPROC; i++)
        freePcb(procp[i]);
    addokbuf("freed 10 entries   \n");

    /* create a 10-element process queue */
    LIST_HEAD(qa);
    if (!emptyProcQ(&qa))
        adderrbuf("emptyProcQ: unexpected FALSE   ");
    addokbuf("Inserting...   \n");
    for (i = 0; i < 10; i++) {
        if ((q = allocPcb()) == NULL)
            adderrbuf("allocPcb: unexpected NULL while insert   ");
        switch (i) {
            case 0:
                firstproc = q;
                break;
            case 5:
                midproc = q;
                break;
            case 9:
                lastproc = q;
                break;
            default:
                break;
        }
        insertProcQ(&qa, q);
    }
    addokbuf("inserted 10 elements   \n");

    if (emptyProcQ(&qa))
        adderrbuf("emptyProcQ: unexpected TRUE");

    /* Check outProc and headProc */
    if (headProcQ(&qa) != firstproc)
        adderrbuf("headProcQ failed   ");
    q = outProcQ(&qa, firstproc);
    if (q == NULL || q != firstproc)
        adderrbuf("outProcQ failed on first entry   ");
    freePcb(q);
    q = outProcQ(&qa, midproc);
    if (q == NULL || q != midproc)
        adderrbuf("outProcQ failed on middle entry   ");
    freePcb(q);
    if (outProcQ(&qa, procp[0]) != NULL)
        adderrbuf("outProcQ failed on nonexistent entry   ");
    addokbuf("outProcQ ok   \n");

    /* Check if removeProc and insertProc remove in the correct order */
    addokbuf("Removing...   \n");
    for (i = 0; i < 8; i++) {
        if ((q = removeProcQ(&qa)) == NULL)
            adderrbuf("removeProcQ: unexpected NULL   ");
        freePcb(q);
    }
    if (q != lastproc)
        adderrbuf("removeProcQ: failed on last entry   ");
    if (removeProcQ(&qa) != NULL)
        adderrbuf("removeProcQ: removes too many entries   ");

    if (!emptyProcQ(&qa))
        adderrbuf("emptyProcQ: unexpected FALSE   ");

    addokbuf("insertProcQ, removeProcQ and emptyProcQ ok   \n");
    addokbuf("process queues module ok      \n");

    addokbuf("checking process trees...\n");

    if (!emptyChild(procp[2]))
        adderrbuf("emptyChild: unexpected FALSE   ");

    /* make procp[1] through procp[9] children of procp[0] */
    addokbuf("Inserting...   \n");
    for (i = 1; i < 10; i++) {
        insertChild(procp[0], procp[i]);
    }
    addokbuf("Inserted 9 children   \n");

    if (emptyChild(procp[0]))
        adderrbuf("emptyChild: unexpected TRUE   ");

    /* Check outChild */
    q = outChild(procp[1]);
    if (q == NULL || q != procp[1])
        adderrbuf("outChild failed on first child   ");
    q = outChild(procp[4]);
    if (q == NULL || q != procp[4])
        adderrbuf("outChild failed on middle child   ");
    if (outChild(procp[0]) != NULL)
        adderrbuf("outChild failed on nonexistent child   ");
    addokbuf("outChild ok   \n");

    /* Check removeChild */
    addokbuf("Removing...   \n");
    for (i = 0; i < 7; i++) {
        if ((q = removeChild(procp[0])) == NULL)
            adderrbuf("removeChild: unexpected NULL   ");
    }
    if (removeChild(procp[0]) != NULL)
        adderrbuf("removeChild: removes too many children   ");

    if (!emptyChild(procp[0]))
        adderrbuf("emptyChild: unexpected FALSE   ");

    addokbuf("insertChild, removeChild and emptyChild ok   \n");
    addokbuf("process tree module ok      \n");

    for (i = 0; i < 10; i++)
        freePcb(procp[i]);


    /* check ASH */
    initASH();
    addokbuf("Initialized active semaphore hash   \n");

    /* check removeBlocked and insertBlocked */
    addokbuf("insertBlocked test #1 started  \n");
    for (i = 10; i < MAXPROC; i++) {
        procp[i] = allocPcb();
        if (insertBlocked(&sem[i], procp[i]))
            adderrbuf("insertBlocked(1): unexpected TRUE   ");
    }
    addokbuf("insertBlocked test #2 started  \n");
    for (i = 0; i < 10; i++) {
        procp[i] = allocPcb();
        if (insertBlocked(&sem[i], procp[i]))
            adderrbuf("insertBlocked(2): unexpected TRUE   ");
    }

    /* check if semaphore descriptors are returned to free list */
    p = removeBlocked(&sem[11]);
    if (insertBlocked(&sem[11], p))
        adderrbuf("removeBlocked: fails to return to free list   ");

    if (insertBlocked(&onesem, procp[9]) == FALSE)
        adderrbuf("insertBlocked: inserted more than MAXPROC   ");

    addokbuf("removeBlocked test started   \n");
    for (i = 10; i < MAXPROC; i++) {
        q = removeBlocked(&sem[i]);
        if (q == NULL)
            adderrbuf("removeBlocked: wouldn't remove   ");
        if (q != procp[i])
            adderrbuf("removeBlocked: removed wrong element   ");
        if (insertBlocked(&sem[i - 10], q))
            adderrbuf("insertBlocked(3): unexpected TRUE   ");
    }
    if (removeBlocked(&sem[11]) != NULL)
        adderrbuf("removeBlocked: removed nonexistent blocked proc   ");
    addokbuf("insertBlocked and removeBlocked ok   \n");

    if (headBlocked(&sem[11]) != NULL)
        adderrbuf("headBlocked: nonNULL for a nonexistent queue   ");
    if ((q = headBlocked(&sem[9])) == NULL)
        adderrbuf("headBlocked(1): NULL for an existent queue   ");
    if (q != procp[9])
        adderrbuf("headBlocked(1): wrong process returned   ");
    p = outBlocked(q);
    if (p != q)
        adderrbuf("outBlocked(1): couldn't remove from valid queue   ");
    q = headBlocked(&sem[9]);
    if (q == NULL)
        adderrbuf("headBlocked(2): NULL for an existent queue   ");
    if (q != procp[19])
        adderrbuf("headBlocked(2): wrong process returned   ");
    p = outBlocked(q);
    if (p != q)
        adderrbuf("outBlocked(2): couldn't remove from valid queue   ");
    p = outBlocked(q);
    if (p != NULL)
        adderrbuf("outBlocked: removed same process twice.");
    if (headBlocked(&sem[9]) != NULL)
        adderrbuf("out/headBlocked: unexpected nonempty queue   ");

    for (i = 0; i < MAXPROC; i++)
        freePcb(procp[i]);

    addokbuf("headBlocked and outBlocked ok   \n");
    addokbuf("ASH module ok   \n");

    /* check Namespaces */
    initNamespaces();
    addokbuf("Initialized Namespaces\n");

    /* check normal namespace (getNamespace) */
    addokbuf("getNamespace test #1 started  \n");
    for (i = 0; i < MAXPROC; i++) {
        procp[i] = allocPcb();
        if (getNamespace(procp[i], NS_PID) != NULL)
            adderrbuf("getNamespace(1): unexpected Namespace   ");
    }

    addokbuf("getNamespace test #1 ok\n");
    addokbuf("addNamespace test #1 started\n");
    pid_ns = allocNamespace(NS_PID);
    if (pid_ns == NULL)
            adderrbuf("Unexpected null on allocNS");
    if (addNamespace(procp[3], pid_ns) != TRUE)
            adderrbuf("addNamespace: Unexpected FALSE");
    if (getNamespace(procp[3], NS_PID) == getNamespace(procp[0], NS_PID))
            adderrbuf("getNamespace: Unexpected root namespace for process 3");
    if (getNamespace(procp[3], NS_PID) != pid_ns)
            adderrbuf("getNamespace: Unexpected namespace for process 3");
    addokbuf("addNamespace: test ok\n");

    addokbuf("addNamespace(2): test started\n");
    /* Change namespace with child */
    insertChild(procp[1], procp[2]);
    addNamespace(procp[1], pid_ns);

    if (getNamespace(procp[2], NS_PID) == NULL)
	    adderrbuf("Child namespace is the root one");
    if (getNamespace(procp[2], NS_PID) != pid_ns)
            adderrbuf("Child namespace is not the one of the parent!");
    addokbuf("addNamespace(2): test ok\n");

    pid_ns2 = allocNamespace(NS_PID);

    addNamespace(procp[1], pid_ns2);

    if (getNamespace(procp[0], NS_PID) != NULL)
            adderrbuf("Root namespace changed!");
    if (getNamespace(procp[1], NS_PID) != pid_ns2)
            adderrbuf("Parent namespace did not changed!");
    if (getNamespace(procp[2], NS_PID) != pid_ns2)
            adderrbuf("Child namespace did not changed!");
    if (getNamespace(procp[3], NS_PID) != pid_ns)
            adderrbuf("Other process namespace changed!");

    addokbuf("Namespace module ok\n");
    addokbuf("So Long and Thanks for All the Fish\n");
    return 0;
}
