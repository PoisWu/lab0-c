#include <stdio.h>
#include "list.h"
#include "queue.h"

int main()
{
    struct list_head *q = q_new();
    element_t *entry;
    q_insert_head(q, "a");
    q_insert_head(q, "b");
    q_insert_head(q, "c");
    q_insert_head(q, "d");
    list_for_each_entry (entry, q, list) {
        printf("%s ", entry->value);
    }
    printf("\n");
    q_reverse(q);
    list_for_each_entry (entry, q, list) {
        printf("%s ", entry->value);
    }
    return 0;
}
