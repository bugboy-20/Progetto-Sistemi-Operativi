#include "types.h"

void initNamespaces();
nsd_t *getNamesapce(pcb_t *p, int type);
int addNamespace(pcb_t *p, nsd_t *ns);
nsd_t *allocNamespace(int type);
void freeNamespace(nsd_t *ns);
