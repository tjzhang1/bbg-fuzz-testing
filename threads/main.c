#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "aht20.h"
#include "circularqueue.h"

static cq_t *q;
static int stop_measure = 0;
static pthread_mutex_t lock;

void *measureall(void *ptr) {
   double hum, temp;
   while(!stop_measure) {
      measure(&hum, &temp);

      //mutually exclusive write
      pthread_mutex_lock(&lock);
      enqueue(q, hum, temp);
      pthread_mutex_unlock(&lock);

      //delay 1s
      sleep(1);
   }
   return NULL;
}

int main(void) {
   double hum, temp;
   int sensorfd;
   pthread_t thread1;
   
   //initialize mutex
   if (pthread_mutex_init(&lock, NULL) != 0) { 
      printf("\n mutex init has failed\n"); 
      return 1; 
   } 
   //initialize data buffer
   q = initbuffer(5);
   //initialize i2c sensor   
   if ( (sensorfd=open(I2C_BUS, O_RDWR)) < 0 )
   {
      perror("Failed to open i2c bus");
      exit(1);
   }   
   if( init_sensor(sensorfd) )
      exit(1);
   //start measurement thread
   pthread_create( &thread1, NULL, measureall, NULL);
   while(getchar() == '\n') {
      //mutually exclusive read
      pthread_mutex_lock(&lock);
      dequeue(q, &hum, &temp);
      pthread_mutex_unlock(&lock);

      printf("%f%%, %f C\n", hum, temp);
   }
   //stop measurement thread
   stop_measure = 1;
   pthread_join( thread1, NULL);
   printf("\nMeasurement stopped\n");
   
   flushbuffer(q);
   close(sensorfd);
   exit(0);
}
