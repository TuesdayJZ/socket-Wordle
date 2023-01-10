// compile: gcc -w Server.c ServerLib.c -o Server
// run: ./Server <port number> <number of players>
#include "ServerLib.h"

#define BUFSIZE 5

int main(int argc, char *argv[]) {
  int servSock; /* Socket descriptor for server */
  int clntSock; /* Socket descriptor for client */
  pid_t processID;
  char ans[BUFSIZE];

  if (argc != 3) /* Test for correct number of arguments */ {
    fprintf(stderr, "Usage: %s <Server Port> <Number of Players>\n", argv[0]);
    exit(1);
  }

  unsigned short ServPort = atoi(argv[1]); /* First arg: local port */
  unsigned int processLimit =
      atoi(argv[2]); /* Second arg:  number of child processes */

  if ((servSock = CreateServerSocket(ServPort)) < 0)
    DieWithError("failed to create server socket");

  selectAns(ans);
  printf("ans: %s\n", ans);

  for (int i = 0; i < processLimit; i++) {
    if ((processID = fork()) < 0)
      DieWithError("fork() failed");
    else if (processID == 0)
      ProcessMain(servSock, ans);
  }
  // exit(0);
}