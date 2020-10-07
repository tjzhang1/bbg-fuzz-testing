#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "tcp.h"

void interface(connfd) {
   char buff[BUF_LEN]; 
   int n; 
   while(1) { 
      //clear buffer
      memset(buff, 0, BUF_LEN); 
      printf("Enter the string : "); 
      n = 0; 
      //receive message from user
      while ((buff[n++] = getchar()) != '\n'); 
      //send message to socket
      write(connfd, buff, BUF_LEN);

      //Show server's reseponse
      memset(buff, 0, BUF_LEN);
      read(connfd, buff, BUF_LEN); 
      printf("From Server : %s", buff); 
      if ((strncmp(buff, "exit", 4)) == 0) { 
         printf("Client Exit...\n"); 
         break; 
      } 
   }
}

int main(int num_args, char **args)
{
   int connfd;

   if(num_args != 3) {
      fprintf(stderr, "error: usage is %s <host> <port>\n", args[0]);
      exit(0);
   }
   
   if( (connfd = open_clientfd(args[1], args[2])) < 0) {
      fprintf(stderr, "error: failed to connect to %s:%s\n", args[1], args[2]);
      exit(1);
   }

   interface(connfd); 
   
   close(connfd);
   exit(0);
}

