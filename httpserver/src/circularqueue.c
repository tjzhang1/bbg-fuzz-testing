#include <stdlib.h>
#include <stdint.h>

#include "circularqueue.h"

cq_t *initbuffer(int num) {
   int i;
   sensordata_t *data, *prev;
   //ensure function was called with nonzero size
   if(num <= 0)
      return NULL;
   
   cq_t *queue = (cq_t *) malloc(sizeof(cq_t));
   //allocate space for the first position in buffer, set as HEAD and TAIL
   data = (sensordata_t *) malloc(sizeof(sensordata_t));
   prev = data;
   queue->HEAD = data;
   queue->TAIL = data;
   //allocate space for the all other positions in buffer until num is reached
   for(i=1; i<num; i++) {
      data = (sensordata_t *) malloc(sizeof(sensordata_t));
      prev->next = data;
      prev = prev->next;
   }
   //link the last element to the first element to make queue circular
   prev->next = queue->HEAD;

   queue->num_filled = 0;
   queue->size = num;
   return queue;
}

void flushbuffer(cq_t *queue) {
   sensordata_t *data, *next;
   int size, i;
   
   size = queue->size;
   data = queue->HEAD;

   next = data->next;
   free(data);
   for(i=1; i<size; i++) {
      data = next;
      next = data->next;
      free(data);
   }

   free(queue);  
}

int checkFull(cq_t *queue) {
   if( queue->num_filled == queue->size )
      return 1;
   return 0;
}

int checkEmpty(cq_t *queue) {
   if( queue->num_filled == 0 )
      return 1;
   return 0;
}

void enqueue(cq_t *queue, double h, double t) {
   sensordata_t *head;

   head = queue->HEAD;
   //if queue is full, increment TAIL so HEAD overwrites oldest data
   if(checkFull(queue))
      queue->TAIL = queue->TAIL->next;
   else
      (queue->num_filled)++;
   //update values of HEAD
   head->humidity = h;
   head->temperature = t;
   //increment HEAD to next position in linked list
   queue->HEAD = head->next;
}

void dequeue(cq_t *queue, double *h, double *t) {
   sensordata_t *tail;

   tail = queue->TAIL;
   //if queue is empty, nothing will happen
   if(checkEmpty(queue)) {
      *h = -1;
      *t = -1;
      return;
   }
   //update passed addresses
   *h = tail->humidity;
   *t = tail->temperature;
   //increment TAIL to next position in linked list
   queue->TAIL = tail->next;
   (queue->num_filled)--;
}
