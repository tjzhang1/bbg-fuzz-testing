#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "http.h"
#include "tcp.h"

/* Sends the first BUF_LEN bytes of msg to the connected socket connfd */
void writeMsg(int connfd, const char* msg) {
   char buff[BUF_LEN];
   
   strcpy(buff, msg);
   write(connfd, buff, BUF_LEN);
}

/* 
 * Generates HTTP response headers for the given status code, status message,
 * and content. Sends the string to connected socket connfd.
 */
int httpResponse(int code, const char *status, char *content, int connfd) {
   char buff[BUF_LEN];

   sprintf(buff, "%s %d %s\r\n", VERSION, code, status);
   sprintf(buff, "%sServer: %s\r\n", buff, SERVER);
   sprintf(buff, "%sContent-Type: text/html\r\n", buff);
   sprintf(buff, "%sContent-Length: %d\r\n\r\n", buff, strlen(content));
   sprintf(buff, "%s%s\r\n", buff, content);
   
   write(connfd, buff, BUF_LEN);
   return 0;
}

