#include "list.h"
#include <stdlib.h>
#include "malloc.h"	

void list_init(list_t *list)
{
    list->head = NULL; 
}

int list_len(list_t *list)
{
    int i = 0; 
    node_t *head = list->head; 
    
    for (; head; head = head->next, i++); 
    return i; 
}


static node_t *new_node(void *data)
{
    node_t *node = (node_t *)mymalloc(SRAMIN,sizeof(node_t)); 
    node->data = data; 
    node->next = NULL; 
    return node; 
}

static int get_index_by_node(list_t *list, node_t *p)
{
    int i; 
    node_t *t = list->head; 
    for (i = 0; t != p; t = t->next, i++); 
    return i; 
}

void list_append(list_t *list, void *data)
{
    node_t *head = list->head, *node = new_node(data); 
    
    if (head){
        for (; head->next; head = head->next); //goto the last node
        head->next = node; 
    }else list->head = node; 
}

int list_pop_tail(list_t *list)
{
    node_t *head = list->head; 
    
    if (!head) return 0; //only 0 eles, pop fail
    
    if (!(head->next)){ //only 1 node , pop it.
        myfree(SRAMIN,head); 
        list->head = NULL; 
        return 1; 
    }
    
    for (; head->next->next; head = head->next); //goto the last second node
    myfree(SRAMIN,head->next); 
    head->next = NULL; 
    return 1; 
}

int list_pop(list_t *list, int pos)
{
    int i = 0; 
    node_t *head = list->head;
	node_t *temp = head; 
    
    if (!head) return 0; //no nodes at all
    
    for (; temp && i != pos; temp = temp->next, i++);  //get the node on position pos
    
    if (!temp) return 0; //unavaliable pos input:go through the list but no position equal pos.temp == NULL means we cycle the list
    
    if (temp == head){ //if pop node is the head node
        list->head = head->next; 
        myfree(SRAMIN,head); 
        return 1; 
    }
    
    for (; head->next != temp; head = head->next); //else goto the pop node's pre node
    
    head->next = temp->next; 
    myfree(SRAMIN,temp); 
    return 1; 
}

int list_iter(list_t *list, void **data_ptr, int *pos_ptr)
{
    static node_t *p = NULL; 
    static int flag = 0; 
    
    if (!flag){ //p init to head node when the first time
        p = list->head; 
        flag = 1; 
    }
    
    if (!p){
        flag = 0;
        return 0; 
    }
    *data_ptr = p->data; //get data 
    *pos_ptr = get_index_by_node(list, p); //get index
    //move next
    p = p->next; 
    return 1; 
}

int list_get(list_t *list, int pos, void **data_ptr)
{
    node_t *head = list->head; 
    int i = 0; 
    
    for (; head; head = head->next, i++){
        if (i == pos) {
            *data_ptr = head->data; 
            return 1; 
        }
    }
    return 0; 
}

int list_set(list_t *list, int pos, void *data)
{
    node_t *t; 
    int i;
    
    for (t = list->head, i = 0; t; t = t->next, i++){
        if (i == pos) {
            t->data = data; 
            return 1; 
        }
    }
    return 0; 
}

void list_extend(list_t *list_a, list_t *list_b)
{
    node_t *t; 
    if (list_a->head == NULL) list_a->head = list_b->head; 
    else{
        for (t = list_a->head; t->next; t = t->next); 
        t->next = list_b->head; 
    }
}

int list_swap(list_t *list, int pos_a, int pos_b)
{
    node_t *t, *a_pre = 0, *b_pre = 0, *a = 0, *b = 0, *head = list->head; 
    int i; 
    
    if (pos_a == 0) a = head; 
    if (pos_b == 0) b = head; 
    
    for (t = head, i = 0; t; t = t->next, i++){
        if (i == pos_a-1) {
            a_pre = t; 
            a = t->next; 
        }
        if (i == pos_b-1) {
            b_pre = t; 
            b = t->next; 
        }
    }
    
    if (!a || !b) return 0; //wrong input position
    if (a == b) return 0; //do not swap the same node
    
    //change pre node's next
    if (a == head){
        list->head = b; 
        b_pre->next = a; 
    }else if (b == head){
        list->head = a; 
        a_pre->next = b; 
    }else{
        a_pre->next = b; 
        b_pre->next = a; 
    }
    
    //change a and b's next
    t = a->next; 
    a->next = b->next; 
    b->next = t; 
    return 1; 
}

void list_reverse(list_t *list)
{
    node_t *q = 0, *t, *m = list->head; 
    for (; m; t = m->next, m->next = q, q = m, m = t); 
    list->head = q; 
}

int list_index(list_t *list, void *data)
{
    node_t *t = list->head; 
    int i = 0; 
    
    for (; t; t = t->next, i++){
        if (t->data == data) return i; 
    }
    return -1; 
}


void list_cycle(list_t *list, void **data_ptr, int * pos_ptr)
{
    static node_t *p = NULL;
    if (!p) p = list->head; 
    
    *data_ptr = p->data;//get data
    *pos_ptr = get_index_by_node(list, p); //get index
    //move
    p = p->next;
}

int list_from_array(list_t *list, void *ptr, int size, int len)
{
    if (list->head) return 0; //list is not empty
    
    int i; 
    node_t *t; 
    
    list->head = new_node(ptr); //init head
    
    for (i = 1, t = list->head; i < len; i++, t = t->next){
        t->next = new_node((char *)ptr+size*i); 
	}
	return 1; 
}

