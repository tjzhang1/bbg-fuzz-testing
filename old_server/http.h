#define VERSION "HTTP/1.0"
#define SERVER "BeagleBone Green"
#define BASE "./source"

/* Sends the first BUF_LEN bytes of msg to the connected socket connfd */
void writeMsg(int connfd, const char* msg) ;

/* 
 * Generates HTTP response headers for the given status code, status message,
 * and content. Sends the string to connected socket connfd.
 */
int httpResponse(int code, const char *status, char *content, int connfd) ;

/*
 * Parses the HTTP request sent by the client and may send an HTTP response
 * indicating an error. Returns -1 if an error occurs, or an integer >= 0 if
 * the request can be handled.
 */
int checkRequest(char *buff, int connfd) ;

