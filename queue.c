#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Create an empty queue */
struct list_head *q_new()
{
    // Always need to check malloc return NULL
    struct list_head *q = malloc(sizeof(struct list_head));
    if (q)
        INIT_LIST_HEAD(q);
    return q;
}
/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    element_t *node, *next;
    list_for_each_entry_safe (node, next, l, list) {
        free(node->value);
        free(node);
    }
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ele = malloc(sizeof(element_t));
    if (!ele)
        return false;

    // yeah I forgot strdup
    ele->value = strdup(s);
    if (!ele->value) {
        free(ele);
        return false;
    }
    list_add(&ele->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ele = malloc(sizeof(element_t));

    if (!ele)
        return false;

    ele->value = strdup(s);
    if (!ele->value) {
        free(ele);
        return false;
    }

    list_add_tail(&ele->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *first_entry = list_first_entry(head, element_t, list);

    if (sp) {
        strncpy(sp, first_entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    list_del(&first_entry->list);
    return first_entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *last_entry = list_last_entry(head, element_t, list);

    if (sp) {
        strncpy(sp, last_entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    list_del(&last_entry->list);
    return last_entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    int size = 0;
    struct list_head *node;
    if (head) {
        list_for_each (node, head) {
            size++;
        }
    }
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    // if head is NULL or empty return false
    if (!head || list_empty(head))
        return false;

    struct list_head *slow = head->next;
    struct list_head *fast = head->next;

    // For loop end when fast hit head
    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }

    element_t *mid_entry = list_entry(slow, element_t, list);
    list_del(slow);
    q_release_element(mid_entry);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/

    if (!head || list_empty(head))
        return false;
    // The list is sorted already!!
    element_t *cur, *next;

    int isdup = 0;
    list_for_each_entry_safe (cur, next, head, list) {
        if (cur->list.next != head && strcmp(cur->value, next->value) == 0) {
            // delete cur
            list_del(&cur->list);
            q_release_element(cur);
            isdup = 1;
        } else if (isdup) {
            list_del(&cur->list);
            q_release_element(cur);
            isdup = 0;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head)
        return;
    struct list_head *cur, *follow, *safe;

    for (cur = head->next, follow = cur->next, safe = follow->next;
         cur != head && follow != head;
         cur = safe, follow = cur->next, safe = follow->next) {
        list_move(cur, follow);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *cur, *backup;
    cur = head;
    do {
        backup = cur->next;
        struct list_head *next = cur->next;
        struct list_head *prev = cur->prev;
        cur->next = prev;
        cur->prev = next;
        cur = backup;
    } while (cur != head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;
    // Need to use the list_cut_position
    // Idea: cut list -> reverse -> inject it back
    int cou = 0;
    struct list_head *cur, *safe, *start = head;
    struct list_head tmp_list;
    INIT_LIST_HEAD(&tmp_list);

    list_for_each_safe (cur, safe, head) {
        if (cou < k - 1) {
            cou++;
            continue;
        }
        // Reverse the element from ]start, cur] <- k elements
        list_cut_position(&tmp_list, start, cur);
        q_reverse(&tmp_list);
        list_splice_tail(&tmp_list, safe);  // splice before safe
        // INIT_LIST_HEAD(&tmp_list);
        start = safe->prev;  // update safe
        cou = 0;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) {}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    // Run reversely
    // How to deal with the first element?
    if (!head || list_empty(head))
        return 0;
    int cou = 0;
    char *str_tmp = list_entry(head->prev, element_t, list)->value;
    struct list_head *cur, *safe;
    for (cur = head->prev, safe = cur->prev; cur != head;
         cur = safe, safe = cur->prev) {
        element_t *cur_entry = list_entry(cur, element_t, list);
        if (strcmp(cur_entry->value, str_tmp) > 0) {
            list_del(cur);
            // q_release_element(
        } else {
            cou++;
            str_tmp = cur_entry->value;
        }
    }
    return cou;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    int cou = 0;
    char *str_tmp = list_entry(head->prev, element_t, list)->value;
    struct list_head *cur, *safe;
    for (cur = head->prev, safe = cur->prev; cur != head;
         cur = safe, safe = cur->prev) {
        element_t *cur_entry = list_entry(cur, element_t, list);
        if (strcmp(cur_entry->value, str_tmp) < 0) {
            list_del(cur);
            // q_release_element(
        } else {
            cou++;
            str_tmp = cur_entry->value;
        }
    }
    return cou;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
