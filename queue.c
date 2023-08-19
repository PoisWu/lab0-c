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
extern void exception_cancel();

void print_list(struct list_head *head)
{
    element_t *node;
    list_for_each_entry (node, head, list) {
        printf("%s ", node->value);
    }
    printf("\n------------------\n");
}
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
    // treat NULL
    exception_cancel();
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


// Merge sorted left and right to head
void *mergeTwoLists(struct list_head *left,
                    struct list_head *right,
                    struct list_head *head,
                    bool descend)
{
    // printf("Before merge---\n");
    // print_list(left);
    // print_list(right);

    if (!left || !right || !head)
        return NULL;
    // struct list_head **ptr = &head;
    INIT_LIST_HEAD(head);
    while (!list_empty(left) && !list_empty(right)) {
        struct list_head *node;
        element_t *entry_left = list_entry(left->next, element_t, list);
        element_t *entry_right = list_entry(right->next, element_t, list);
        node = (strcmp(entry_left->value, entry_right->value) <= 0) != descend
                   ? left
                   : right;
        list_move_tail(node->next, head);
    }
    list_splice_tail(left, head);
    list_splice_tail(right, head);
    // printf("after merge\n");
    // print_list(head);
    // getchar();
    return head;
}


void merge_sort(struct list_head *head, bool descend)
{
    // devide and conqueur
    // len(head) == 0 or 1
    if (!head || list_empty(head) || head->next->next == head)
        return;

    struct list_head left, right;
    INIT_LIST_HEAD(&left);
    INIT_LIST_HEAD(&right);

    struct list_head *slow = head->next;
    struct list_head *fast = head->next;

    // For loop end when fast hit head
    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }
    list_cut_position(&left, head, slow->prev);
    list_splice(head, &right);
    // print_list(&left);
    // print_list(&right);
    // getchar();
    merge_sort(&left, descend);
    merge_sort(&right, descend);
    // print_list(&left);
    // print_list(&right);
    // getchar();
    INIT_LIST_HEAD(head);
    mergeTwoLists(&left, &right, head, descend);
}

// Implementation of bubble_sort
void bubble_sort(struct list_head *head, bool descend)
{
    if (!head)
        return;
    int n = q_size(head);
    for (int i = 0; i < n - 1; i++) {
        element_t *cur = list_entry(head->next, element_t, list);
        element_t *nxt = list_entry(cur->list.next, element_t, list);
        for (int j = 0; j < n - i - 1; j++) {
            if ((strcmp(cur->value, nxt->value) > 0) != descend) {
                list_move(&cur->list, &nxt->list);
            } else {
                cur = nxt;
            }
            nxt = list_entry(cur->list.next, element_t, list);
        }
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    merge_sort(head, descend);
}

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
            q_release_element(cur_entry);
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
            q_release_element(cur_entry);
        } else {
            cou++;
            str_tmp = cur_entry->value;
        }
    }
    return cou;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
/**
 * queue_contex_t - The context managing a chain of queues
 * @q: pointer to the head of the queue
 * @chain: used by chaining the heads of queues
 * @size: the length of this queue
 * @id: the unique identification number
 */


int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    exception_cancel();
    if (!head || list_empty(head))
        return 0;
    struct list_head queue_tmp;
    int n = q_size(head);
    // if nb(queue) > 1
    struct list_head *cur_back = head->prev;
    while (n > 1) {
        for (int i = 0; i < n / 2; i++) {
            struct list_head *cur_front = head->next;
            struct list_head *q1 =
                list_entry(cur_front, queue_contex_t, chain)->q;
            struct list_head *q2 =
                list_entry(cur_back, queue_contex_t, chain)->q;
            INIT_LIST_HEAD(&queue_tmp);
            list_splice(q1, &queue_tmp);
            mergeTwoLists(q2, &queue_tmp, q1, descend);
            cur_front = cur_front->next;
            cur_back = cur_back->prev;
        }
        n = (n + 1) / 2;
    }
    // while (head->next->next != head) {
    //     // printf("Begin: size of chain %d\n", q_size(head));
    //     for (cur = head->next, fol = cur->next, safe = fol->next;
    //          cur != head && fol != head;
    //          cur = safe, fol = cur->next, safe = fol->next) {
    //         struct list_head *q1 = list_entry(cur, queue_contex_t, chain)->q;
    //         struct list_head *q2 = list_entry(fol, queue_contex_t, chain)->q;
    //         // merge q1,q2 into q1;
    //         if (!q1)
    //             // list_del(cur);
    //         if (!q2)
    //             // list_del(fol);
    //
    //         if (q1 && q2) {
    //             INIT_LIST_HEAD(&queue_tmp);
    //             // move q1 -> queue_tmp
    //             list_splice(q1, &queue_tmp);
    //             // mergeTwoList(q2,queue_tmp, q1)
    //             mergeTwoLists(q2, &queue_tmp, q1, descend);
    //             // del(q2)
    //             // list_del(fol);
    //         }
    //         // print_list(q1);
    //         // print_list(q2);
    //         // printf("After: size of chain %d\n", q_size(head));
    //         // getchar();
    //     }
    // }
    printf("End: size of chain %d\n", q_size(head));
    struct list_head *queue = list_entry(head->next, queue_contex_t, chain)->q;
    print_list(queue);
    getchar();
    return q_size(queue);
}
