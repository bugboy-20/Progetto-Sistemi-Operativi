#ifndef _TYPES_H
#define _TYPES_H
//#define NULL ((void *)0)

#ifndef bool
typedef int bool;
#endif

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

struct list_head {
	struct list_head *next, *prev;
};

#ifndef u32
typedef unsigned int u32;
#endif
struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};



#endif
