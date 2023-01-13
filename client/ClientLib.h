#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>      /* for fcntl() */
#include <errno.h>      /* for errno */

#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define BUFSIZE 5    /* Size of receive buffer */

void DieWithError(char *errorMessage); /* Error handling function */
int isAlp_toLower(char *str);          /* Check if string is alphabet and if its upper, change to lower */
void gamePlay(int sock);               /* game loop */
void gameIntro(int sock);              /* Wordle introducton and select how many trial */