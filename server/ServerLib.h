#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>     /* for close() */

void DieWithError(char *errorMessage);  /* Error handling function */
int CreateServerSocket(unsigned short port); /* Create  server socket */
int AcceptConnection(int servSock);  /* Accept connection request */
void gameMaster(int sock, char *ans, int pid); /* Game master */
void selectWordle(char *wordle, char **words); /* Select wordle */
void readWords(char **words); /* Read words from file */
void ProcessMain(int servSock, char *ans);
void multi_wait(int processCount);