#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "tcp.h"
#include "aht20.h"
#include "http.h"

/* 
 * Performs different actions based on integer id, which was provided by the
 * checkRequest function in http.h. New server features may be added here.
 */
void interface(int connfd, int id) 
{ 
   double humidity, temp;
   char content[BUF_LEN];

   switch(id) {
      case 0:
         measure(&humidity, &temp);
         sprintf(content, "Humidity: %f%%\r\nTemperature: %f C", humidity, temp);
         httpResponse(200, "OK", content, connfd);
         break;
      case 1:
         if(soft_reset())
            httpResponse(500, "Internal Server Error", "The sensor failed to reset.", connfd);
         else
            httpResponse(200, "OK", "Sensor reset successfully.", connfd);
         break;
      case 2:
         if(check_status())
            httpResponse(200, "OK", "Sensor is busy.", connfd);
         else
            httpResponse(200, "OK", "Sensor is idle.", connfd);
         break;
      default:
         httpResponse(500, "Internal Server Error", "Something went wrong.", connfd);
   }
}

int main(int num_args, char **args)
{
   int listenfd, connfd, sensorfd;
   socklen_t clientlen;
   struct sockaddr_storage clientaddr;
   char client_hostname[NI_MAXHOST], client_servicename[NI_MAXSERV];  //buffers to store client name info

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
   if ( (sensorfd=open(I2C_BUS, O_RDWR)) < 0 )
   {
      perror("Failed to open i2c bus");
      exit(1);
   }   
   if( init_sensor(sensorfd) )
      exit(1);


   //server loop
   while(1) {
      char buff[BUF_LEN];  //used to store messages for reads
      int req_num;

      clientlen = sizeof(struct sockaddr_storage);
      if( (connfd = accept(listenfd, (SA *)&clientaddr, &clientlen)) < 0 ) {
         fprintf(stderr, "accept error\n");
         exit(1);
      }
      getnameinfo((SA *)&clientaddr, clientlen, client_hostname, NI_MAXHOST, client_servicename, NI_MAXSERV, 0);
      printf(">> Server: Connection established with %s:%s!\n", client_hostname, client_servicename); 
      //read client's message
      read(connfd, buff, BUF_LEN); 
      printf(">> From client: %s\n", buff);

      //process client's request
      if( (req_num = checkRequest(buff, connfd)) >= 0 ) {
         interface(connfd, req_num);
      }
      close(connfd);
   }
   
   close(listenfd);
   return 0;
}
