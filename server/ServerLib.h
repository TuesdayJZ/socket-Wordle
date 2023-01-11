#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define BUFSIZE 5    /* Size of receive buffer */
#define WORDS 14855

void DieWithError(char *errorMessage);          /* Error handling function */
int CreateServerSocket(unsigned short port);    /* Create  server socket */
int AcceptConnection(int servSock);             /* Accept connection request */
void gameMaster(int sock, char *ans, int pid, char **words);  /* Game master */
void selectWordle(char *wordle, char **words);  /* Select wordle */
void readWords(char **words);                   /* Read words from file */
void ProcessMain(int servSock, char *ans, char **words);
void multi_wait(int processCount);