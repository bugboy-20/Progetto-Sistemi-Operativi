#define MAX_TYPES 20

typedef struct pcb_t
{
    /* process queue */
    struct list_head p_next;
    /* process tree fields */
    struct pcb_t *p_parent;   /* ptr to parent */
    struct list_head p_child; /* children list */
    struct list_head p_sib;   /* sibling list */
    /* process status information */
    state_t p_s;                    /* processor state */
    cpu_t p_time;                   /* cpu time used by proc */
    int *p_semAdd;                  /* ptr to semaphore on which proc is blocked */
    nsd_t *p_namespaces[MAX_TYPES]; /* ACTIVE namespace for each type */
} pcb_t, *pcb_PTR;

/* semaphore descriptor data structure */
typedef struct semd_t
{
    struct hlist_node s_link;
    struct list_head s_freeLink;
    /* Semaphore key */
    int *s_key;
    /* PCBs blocked on the semaphore */
    struct list_head s_procQ;
} semd_t;

/* namespace descriptor data structure*/
typedef struct nsd_t
{
    int n_type;
    struct list_head n_link;
} nsd_t;