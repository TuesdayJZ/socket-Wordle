#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

void DieWithError(char *errorMessage);  /* Error handling function */
void HandleClient(int clntSocket);   /* client handling function */
int CreateServerSocket(unsigned short port); /* Create  server socket */
int AcceptConnection(int servSock);  /* Accept connection request */