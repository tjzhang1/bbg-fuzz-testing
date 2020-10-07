#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "tcp.h"

/*
 *Establishes a connection with a server running on hostname and listening for
 * connection requests on port number port.
 *Returns a connected socket file descriptor ready for input/output.
 */
int open_clientfd(char *hostname, char *port)
{
   int clientfd;
   struct addrinfo hints, *head, *n;

   /* Get a list of potential server addresses using getaddrinfo() */
   memset(&hints, 0, sizeof(struct addrinfo));  //set all values in hints to 0
   hints.ai_socktype = SOCK_STREAM;
   //getaddrinfo will return IPv4 or IPv6 address only if local host is configured for them
   hints.ai_flags = AI_ADDRCONFIG;
   getaddrinfo(hostname, port, &hints, &head);  //finds linked list of addrinfo and stores at head

   /* Walk the list of addresses until we can connect to one. */
   for (n = head; n; n = n->ai_next)
   {
      //attempt to create socket descriptor
      if( (clientfd = socket(n->ai_family, n->ai_socktype, n->ai_protocol)) < 0 )
         continue;
      
      //Socket creation succeeded; now attempt to connect to server
      if( connect(clientfd, n->ai_addr, n->ai_addrlen) < 0 )
      {
         close(clientfd);
         continue;
      }
      
      //At this point, connection is successful and we can return the fd
      freeaddrinfo(head);
      return clientfd;
   }

   //no connections could be made
   return -1;
}

/*
 *Creates a listening descriptor ready to receive connection requests.
 *Returns the listening descriptor ready to receive connection requests on port
 * port.
 */
int open_listenfd(char *port)
{
   int listenfd, optval=1;
   struct addrinfo hints, *head, *n;
   
   /* Get a list of potential server addresses using getaddrinfo() */
   memset(&hints, 0, sizeof(struct addrinfo));  //set all values in hints to 0
   hints.ai_socktype = SOCK_STREAM;
   //getaddrinfo will return IPv4 or IPv6 address only if local host is configured for them
   //Also, the passive flag lets getaddrinfo show socket addresses that server can use for listening
   hints.ai_flags = AI_ADDRCONFIG | AI_PASSIVE;
   getaddrinfo(NULL, port, &hints, &head);  //finds linked list of addrinfo and stores at head

   /* Walk the list of addresses until we can bind to one. */
   for (n = head; n; n = n->ai_next)
   {
      //attempt to create a socket descriptor
      if( (listenfd = socket(n->ai_family, n->ai_socktype, n->ai_protocol)) < 0 )
         continue;
      
      //configure socket to ignore 30s refresh delay
      setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

      //Socket creation succeeded; now attempt to bind the descriptor to the address
      if( bind(listenfd, n->ai_addr, n->ai_addrlen) < 0 )
      {
         close(listenfd);
         continue;
      }
      
      //Bind successful; now make fd a listening socket
      if( listen(listenfd, MAX_BACKLOG) < 0 )
      {
         close(listenfd);
         continue;
      }
      
      //At this point, bind is successful and we can return the fd
      freeaddrinfo(head);
      return listenfd;
   }
   
   //no addresses were bound
   return -1;
}
