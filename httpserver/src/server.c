/*******************System Includes**************************************************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

/******************Local Includes***************************************************************/
#include "tcp.h"
#include "aht20.h"
#include "http.h"
#include "circularqueue.h"
#define RING_BUF_LEN 25

/*********************Bug switch******************************************************/
#define BUF_OVRFLW 1  //0: stack-based, 1: heap-based
#define PRINTF_BUG 0  //0: disabled, 1: enabled
#define UNSAFE_READ 0  //0: disabled, 1: enabled
/* Null ptr dereference and double free are always enabled. */

/*******************Global Variables*****************************************************/
static cq_t *queue;
static int stopMeasureFlag = 0;
static pthread_mutex_t lock;
static int stopServerFlag = 0;
static float calibrateT = 1.0;
static float calibrateH = 1.0;
/*******************Server Prototypes****************************************************/
void *measureall(void *ptr);
int checkRequest(char *buff, int connfd);
void checkPath(int connfd, char *file_path, char *args); 
/****************Buggy Functions*********************************************************/
//vulnerable to stack-based overflow
void getMeasurements(int connfd, int num);
//vulnerable to heap-based overflow
void getMeasurementsHEAP(int connfd, int num);
//vulnerable to format-string bug
void buggyPrint(char *buf) {
   printf(buf);
}
//function performs double free
void doubleFree(void) {
   char *ptr = malloc(BUF_LEN);
   free(ptr);
   free(ptr);
}
//function dereferences a NULL pointer
void nullDereference(void) {
   char *ptr = NULL;
   ptr[0] = 'a';
}

/******************Main****************************************************************/
int main(int num_args, char **args)
{
   int listenfd=-1, connfd=-1, sensorfd=-1;
   socklen_t clientlen;
   struct sockaddr_storage clientaddr;
   char client_hostname[NI_MAXHOST], client_servicename[NI_MAXSERV];  //buffers to store client name info
   pthread_t thread1;
   char buff[BUF_LEN];  //used to store messages for reads

   //check usage
   if(num_args != 2) {
      fprintf(stderr, "error: usage is %s <port>\n", args[0]);
      exit(0);
   }
   //open socket
   if( (listenfd = open_listenfd(args[1])) < 0) {
      fprintf(stderr, "error: failed to open port %s for listening\n", args[1]);
      exit(1);
   }
   //initialize i2c sensor   
   if ( (sensorfd=open(I2C_BUS, O_RDWR)) < 0 ) {
      perror("Failed to open i2c bus");
      exit(1);
   }   
   if( init_sensor(sensorfd) )
      exit(1);
   //intialize data buffer
   queue = initbuffer(RING_BUF_LEN);
   //initialize mutex
   if (pthread_mutex_init(&lock, NULL) != 0) { 
      printf("\n mutex init has failed\n"); 
      exit(1); 
   }
   //start measurement thread
   pthread_create( &thread1, NULL, measureall, NULL);

   //server loop
   while(1) {
      memset(buff, 0, BUF_LEN);
      clientlen = sizeof(struct sockaddr_storage);
      if( (connfd = accept(listenfd, (SA *)&clientaddr, &clientlen)) < 0 ) {
	 int errsv = errno;
         fprintf(stderr, "accept error: %s\n", strerror(errsv));
         break;
      }
      getnameinfo((SA *)&clientaddr, clientlen, client_hostname, NI_MAXHOST, client_servicename, NI_MAXSERV, 0);
      //printf(">> Server: Connection established with %s:%s!\n", client_hostname, client_servicename); 
      //read client's message
#if UNSAFE_READ
      read(connfd, buff, BUF_LEN);
#else
      read(connfd, buff, BUF_LEN-1); //last bit of buff remains 0
#endif

#if PRINTF_BUG
      printf(">> From client: ");
      buggyPrint(buff);      
      printf("\n");
#else
      printf(">> From client: %s\n", buff);
#endif

      //process client's request
      checkRequest(buff, connfd);
      close(connfd);
      if(stopServerFlag)
         break;
   }
   //stop measurement thread
   stopMeasureFlag = 1;
   pthread_join( thread1, NULL );
   //deallocate buffer
   flushbuffer(queue);
   //close opened fd
   close(sensorfd);
   close(listenfd);
   return 0;
}
/********************************************End Main********************************************/

/* Function that reads sensor values every second, to be used in a thread */
void *measureall(void *ptr) {
   double hum, temp;
   while(!stopMeasureFlag) {
      measure(&hum, &temp);
      hum *= calibrateH;
      temp *= calibrateT;

      //mutually exclusive write
      pthread_mutex_lock(&lock);
      enqueue(queue, hum, temp);
      pthread_mutex_unlock(&lock);

      //delay 1s
      sleep(1);
   }
   return NULL;
}

/*
 * Parses the HTTP request sent by the client and may send an HTTP response
 * indicating an error. Returns -1 if an error occurs, or an integer >= 0 if
 * the request can be handled.
 */
int checkRequest(char *buff, int connfd) {
   char *req, method[BUF_LEN], URI[BUF_LEN], protocol[BUF_LEN];
//   char *headers;

   //parse request; get the first line
   if( !(req = strtok(buff, "\r\n")) ) {
      httpResponse(400, "Bad request", "Request could not be understood by the server.", connfd);
      return -1;
   }
   //populate the request info
   if( sscanf(req, "%s %s %s", method, URI, protocol) != 3) {
      httpResponse(400, "Bad request", "Request could not be understood by the server.", connfd);
      return -1;
   }

   //populate the headers
//   headers = strtok(NULL, "\r\n");

   //check protocol
   if( strcmp(protocol, "HTTP/1.0") && strcmp(protocol, "HTTP/1.1") ) {
      httpResponse(505, "HTTP version not supported", "Server does not support version in request.", connfd);
      return -1;
   }

   //check method
   if( strcmp(method, "GET") == 0 ) { 
      char file_path[BUF_LEN];
      char *args;
      //initialize file path
      strcpy(file_path, BASE);
      strcat(file_path, URI);
      //initialize arguments
      strtok(URI, "?");
      args = strtok(NULL, "?");
      
      //check file
      checkPath(connfd, file_path, args);
   }
   else {
      httpResponse(501, "Not implemented", "Server does not support the request method.", connfd);
      return -1;
   }
   return 0;
}

/*
 * Checks the file name requested by the client. Performs the assigned function
 * and sends a corresponding response to the client.
 */
void checkPath(int connfd, char *file_path, char *args) {
   int numMeasurements;   

   //default path
   if( !strcmp(file_path, "./source/") || !strcmp(file_path, "./source/sensor") ) {
      getMeasurements(connfd, 1);
   }
   //access sensor readings
   else if( !strncmp(file_path, "./source/sensor", 15) && args ) {
      //command to reset
      if( !strcmp(args, "reset") ) {
         calibrateT = 1.0;
         calibrateH = 1.0;
         if(soft_reset() || write_init_req())
            httpResponse(500, "Internal Server Error", "The sensor failed to reset.", connfd);
         else
            httpResponse(200, "OK", "Sensor reset successfully.", connfd);
      }
      //command to check status
      else if( !strcmp(args, "status") ) {
         if(check_status())
            httpResponse(200, "OK", "Sensor is busy.", connfd);
         else
            httpResponse(200, "OK", "Sensor is idle.", connfd);
      }
      //adjust calibration settings
      else if( !strncmp(args, "calibrate", 9) ) {
         if( sscanf(args, "calibrate&humidity&%f", &calibrateH) || sscanf(args, "calibrate&temperature&%f", &calibrateT) )
            httpResponse(200, "OK", "Calibration complete.", connfd);
         else
            httpResponse(400, "Bad Request", "Server did not understand the specified arguments.", connfd);
      }
      //command to get measurements
      else if( sscanf(args, "%d", &numMeasurements) ) {
         getMeasurements(connfd, numMeasurements);
      }
      else {
         httpResponse(400, "Bad Request", "Server did not understand the specified arguments.", connfd);
      }
   }
   //close server
   else if( !strcmp(file_path, "./source/close") ) {
      stopServerFlag = 1;
      httpResponse(200, "OK", "Server is shut down.", connfd);
   }
   //double free bug
   else if( !strncmp(file_path, "./source/\\/\\/", 13) ) {
      doubleFree();
   }
   //Null dereference bug
   else if( !strncmp(file_path, "./source/NULL", 13) ) {
      nullDereference();
   }
   else {
      httpResponse(404, "Not found", "Server could not find the requested file.", connfd);
   }
   return;
}

/*
 * Generates a list of measurements to be sent to the client. Uses a local
 * array of fixed length and is susceptible to stack-based buffer overflow
 */
void getMeasurements(int connfd, int num) {
#if BUF_OVRFLW
   getMeasurementsHEAP(connfd, num);
#else
   double hum, temp;
   char content[BUF_LEN];
   int n;
   
   memset(content, 0, BUF_LEN);
   for(n=0; n<num; n++) {
      //mutually exclusive read
      pthread_mutex_lock(&lock);
      dequeue(queue, &hum, &temp);
      pthread_mutex_unlock(&lock);

      sprintf(content, "%s%d. Humidity: %.3f%%\r\n\tTemp: %.3f C\r\n", content, n+1, hum, temp);
   }
   httpResponse(200, "OK", content, connfd);
#endif
}
/*
 * Generates a list of measurements to be sent to the client. Uses an allocated
 * array of fixed length and is susceptible to heap-based buffer overflow
 */
void getMeasurementsHEAP(int connfd, int num) {
   double hum, temp;
   char *content;
   int n;

   content=(char *)malloc(BUF_LEN);
   memset(content, 0, BUF_LEN);

   for(n=0; n<num; n++) {
      //mutually exclusive read
      pthread_mutex_lock(&lock);
      dequeue(queue, &hum, &temp);
      pthread_mutex_unlock(&lock);

      sprintf(content, "%s%d. Humidity: %.3f%%\r\n\tTemp: %.3f C\r\n", content, n+1, hum, temp);
   }
   httpResponse(200, "OK", content, connfd);

   free(content);
}


