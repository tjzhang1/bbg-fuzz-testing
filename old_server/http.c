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
   sprintf(buff, "%sContent-Type: text/plain\r\n", buff);
   sprintf(buff, "%sContent-Length: %d\r\n\r\n", buff, strlen(content));
   sprintf(buff, "%s%s\r\n", buff, content);
   
   write(connfd, buff, BUF_LEN);
   return 0;
}

/*
 * Parses the HTTP request sent by the client and may send an HTTP response
 * indicating an error. Returns -1 if an error occurs, or an integer >= 0 if
 * the request can be handled.
 */
int checkRequest(char *buff, int connfd) {
   char *req, method[BUF_LEN], URI[BUF_LEN], protocol[BUF_LEN];
   char *headers;

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
   headers = strtok(NULL, "\r\n");

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
      if( !strcmp(file_path, "./source/") || !strcmp(file_path, "./source/sensor") ) {
         return 0;
      }
      else if( !strncmp(file_path, "./source/sensor", 15) && args ) {
         if( !strcmp(args, "reset") )
            return 1;
         else if( !strcmp(args, "status") )
            return 2;
         else
            return 0;
      }
      else {
         httpResponse(404, "Not found", "Server could not find the requested file.", connfd);
         return -1;
      }
   }
   else {
      httpResponse(501, "Not implemented", "Server does not support the request method.", connfd);
      return -1;
   }
}
