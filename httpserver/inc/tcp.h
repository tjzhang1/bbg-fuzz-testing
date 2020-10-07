#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>

#define MAX_BACKLOG 0
#define SA struct sockaddr
#define BUF_LEN 512

/*
 *Establishes a connection with a server running on hostname and listening for
 * connection requests on port number port.
 *Returns a connected socket file descriptor ready for input/output.
 */
int open_clientfd(char *hostname, char *port);

/*
 *Creates a listening descriptor ready to receive connection requests.
 *Returns the listening descriptor ready to receive connection requests on port
 * port.
 */
int open_listenfd(char *port);
