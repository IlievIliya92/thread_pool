/******************************** INCLUDE FILES *******************************/
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "queue.h"  /* queue class *
/*********************************** TYPEDEFS *********************************/
typedef struct _node_t
{
    int value;
    struct _node_t *next;
} node_t;

struct _queue_t
{
    node_t *head;
    node_t *tail;
    int size;
};


// Returns a pointer to a new empty queue on the heap
queue_t *queue_new()
{
    queue_t *self = malloc(sizeof(queue_t));
    
    self->head = NULL;
    self->tail = NULL;
    self->size = 0;
    
    return self;
}

// Destroys the entire queue by freeing the memory allocated to store all Node 
// structs in the linked list and the Queue struct itself.
void queue_destroy(queue_t *queue)
{
    node_t *current_node = queue->head;
    
    // The loop will stop when we reach the tail of the list with current_node.
    while (current_node != NULL)
    {
        node_t *temp = current_node;
        current_node = current_node->next;
        free(temp);
    }
    free(queue);
}

int size(queue_t *self)
{
    return self->size;
}


// Returns true if the queue is empty and false if it is not
int is_empty(queue_t *self)
{
    return (self->size == 0);
}


int peek(queue_t *self, int *value)
{
    if (is_empty(self))
    {
        return -1;
    }
    
    *value = self->head->value;
    return 0;
}


// Enqueues the value onto the queue (inserts the value onto the queue).  
void enqueue(queue_t *self, int value)
{
    node_t *new_node = malloc(sizeof(node_t));
    
    new_node->value = value;
    new_node->next = NULL;
    
    // If the queue is empty then this is the only node, and we can set both head
    // and tail to point to this new node.
    if (is_empty(self))
    {
        self->head = new_node;
        self->tail = new_node;
    }
    // Otherwise set the current tail node to point to this new node (instead of 
    // NULL) and update the Queue struct's tail pointer to point to this new Node.
    else
    {
        self->tail->next = new_node;
        self->tail = new_node;
    }
    
    self->size++;
}

int dequeue(queue_t *self, int *value)
{
    if (is_empty(self))
    {
        return -1;
    }
    *value = self->head->value;
    
    node_t *old_head = self->head;
    if (self->size == 1)
    {
        self->head = NULL;
        self->tail = NULL;
    }
    else
    {
        self->head = self->head->next;
    }

    free(old_head);
    self->size--;
    
    return 0;
}
