#ifndef _QUEUE_H_
#define _QUEUE_H_

/******************************** INCLUDE FILES *******************************/
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************** DEFINES **********************************/

/*********************************** TYPEDEFS *********************************/
typedef struct _queue_t queue_t;

/************************** INTERFACE DATA DEFINITIONS ************************/
queue_t *queue_new();
void queue_destroy(queue_t *self);

int size(queue_t *self);
int is_empty(queue_t *self);
int peek(queue_t *self, int *value);
void enqueue(queue_t *self, int value);
int dequeue(queue_t *self, int *value);
    
/************************* INTERFACE FUNCTION PROTOTYPES **********************/
#ifdef __cplusplus
}
#endif
#endif /* _QUEUE_H_ */
