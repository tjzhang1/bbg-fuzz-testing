#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "tcp.h"
#include "http.h"

/* Generates an HTTP request and sends to connected socket at connfd */
void interface(int connfd, char buff[BUF_LEN]) {
   //send message to socket
   writeMsg(connfd, buff);

   memset(buff, 0, BUF_LEN);
   //Show server's reseponse
   read(connfd, buff, BUF_LEN); 
   printf(">> From Server: %s\r\n", buff); 
}

int main(int num_args, char **args)
{
   int connfd, n;
   char buff[BUF_LEN];

   if(num_args != 3) {
      fprintf(stderr, "error: usage is %s <host> <port>\n", args[0]);
      exit(0);
   }
   
   if( (connfd = open_clientfd(args[1], args[2])) < 0) {
      fprintf(stderr, "error: failed to connect to %s:%s\n", args[1], args[2]);
      exit(1);
   }

   printf(">> type your HTTP request: ");
   n=0;
   while( (buff[n++] = getchar()) != '\n' && n<BUF_LEN) ;
   buff[n-1] = 0;

   interface(connfd, buff); 
   
   close(connfd);
   exit(0);
}

