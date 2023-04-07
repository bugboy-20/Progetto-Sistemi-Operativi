#include <stdlib.h>
#include <stdio.h>
#include <hashtable.h>
#include <list.h>

DEFINE_HASHTABLE(hdata, 16);

LIST_HEAD(ldata);

struct testitem {
	int value;
	struct hlist_node hash;
	struct list_head list;
};

int hash_test(int argc, char **argv)
{
	int i;
	int idx;
	struct testitem *item;

	printf("Hash test\n");

	for (i = 1 ; i < argc; ++i) {
		item = calloc(1, sizeof(struct testitem));
		if (item == NULL) {
			return -1;
		}

		idx = atoi(argv[i]);

		item->value = idx * idx;

		hash_add(hdata, &item->hash, idx);
	}

	hash_for_each_possible(hdata, item, hash, 0) {
		printf("possible: %p\n", item);
	}

	hash_for_each(hdata, idx, item, hash) {
		printf("%d: %d\n", idx, item->value);
	}

	return 0;
}

int list_test(int argc, char **argv)
{
	return 0;
}

int main(int argc, char **argv)
{
	if (hash_test(argc, argv) != 0) {
		perror("hash_test");
		return 1;
	}
	if (list_test(argc, argv) != 0) {
		perror("list_test");
		return 1;
	}

	return 0;
}
