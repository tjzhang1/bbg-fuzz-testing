#include <stdint.h>

typedef struct sensordata {
   double temperature;
   double humidity;
   struct sensordata *next;
} sensordata_t;

typedef struct circularqueue {
   sensordata_t *HEAD;
   sensordata_t *TAIL;
   uint32_t num_filled;
   uint32_t size;
} cq_t;


cq_t *initbuffer(int num);

void flushbuffer(cq_t *queue) ;

int checkFull(cq_t *queue) ;

int checkEmpty(cq_t *queue) ;

void enqueue(cq_t *queue, double h, double t) ;

void dequeue(cq_t *queue, double *h, double *t) ;
