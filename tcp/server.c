#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "tcp.h"

void interface(int connfd) 
{ 
   int n;
   char buff[BUF_LEN];  //used to store messages for reads

   while (1) { 
      memset(buff, 0, BUF_LEN);  //clear all values in buffer
      //read client's message
      read(connfd, buff, BUF_LEN); 
      printf("From client: %s\t To client : ", buff); 
      //generate response
      memset(buff, 0, BUF_LEN);  //clear all values in buffer
      n = 0;
      // copy server message in the buffer 
      while ((buff[n++] = getchar()) != '\n');
      write(connfd, buff, BUF_LEN);
      // if client msg contains "Exit" then server breaks connection 
      if (strncmp("exit", buff, 4) == 0) { 
         printf("Server Exit...\n"); 
         break; 
      }
   }
}

int main(int num_args, char **args)
{
   int listenfd, connfd;
   socklen_t clientlen;
   struct sockaddr_storage clientaddr;
   char client_hostname[NI_MAXHOST], client_servicename[NI_MAXSERV];  //buffers to store client name info

   if(num_args != 2) {
      fprintf(stderr, "error: usage is %s <port>\n", args[0]);
      exit(0);
   }
   
   if( (listenfd = open_listenfd(args[1])) < 0) {
      fprintf(stderr, "error: failed to open port %s for listening\n", args[1]);
      exit(1);
   }
   
   while(1) {
      clientlen = sizeof(struct sockaddr_storage);
      if( (connfd = accept(listenfd, (SA *)&clientaddr, &clientlen)) < 0 ) {
         fprintf(stderr, "accept error\n");
         exit(1);
      }
      getnameinfo((SA *)&clientaddr, clientlen, client_hostname, NI_MAXHOST, client_servicename, NI_MAXSERV, 0);
      printf("Server says: Connection established with %s:%s!\n", client_hostname, client_servicename); 

      interface(connfd); //process messages from client
      close(connfd);
   }
   
   close(listenfd);
   return 0;
}
