// compile: gcc -w Server.c ServerLib.c -o Server
// run: ./Server <port number> <number of players>
#include "ServerLib.h"

#define BUFSIZE 5
#define MAXPLAYERS 10

int main(int argc, char *argv[]) {
  int servSock; /* Socket descriptor for server */
  int clntSock; /* Socket descriptor for client */
  pid_t processID;
  int processCount, processLimit;
  char ans[BUFSIZE];

  if (argc > 3 || argc < 2) /* Test for correct number of arguments */ {
    fprintf(stderr, "Usage: %s <Server Port> (<Number of Players>)\n", argv[0]);
    exit(1);
  }

  if(strtol(argv[1], NULL, 10) < 1){
    fprintf(stderr, "Server Port should be a number.\n");
    exit(1);
  }

  unsigned short ServPort = atoi(argv[1]); /* First arg: local port */
  if (argc == 2) processLimit = 1;
  else {
    if (strtol(argv[2], NULL, 10) < 1 || strtol(argv[2], NULL, 10) > MAXPLAYERS) {
      fprintf(stderr, "Number of players should be posotive number below %d.\n", MAXPLAYERS);
      exit(1);
    }
    processLimit = atoi(argv[2]);
  } /* Second arg:  number of child processes */

  if ((servSock = CreateServerSocket(ServPort)) < 0)
    DieWithError("failed to create server socket");

  printf("You can play with %d players.\n", processLimit);
  printf("The server can be closed anytime by pressing Ctrl+C.\n\n");

  if (processLimit == 1) {
    selectAns(ans);
    printf("ans: %s\n", ans);
    ProcessMain(servSock, ans);
    exit(EXIT_SUCCESS);
  }

  printf("Do you want to play with the same answer? [Y/n] : ");

  while (1) {
    scanf("%s", ans);
    if (strcmp(ans, "Y") != 0 && strcmp(ans, "y") != 0 && strcmp(ans, "N") != 0 && strcmp(ans, "n") != 0) {
      printf("Enter Y/n : ");
      continue;
    } else {
      break;
    }
  }
  if (strcmp(ans, "Y") == 0 || strcmp(ans, "y") == 0) {
    selectAns(ans);
    printf("ans: %s\n", ans);
    for (processCount = 0; processCount < processLimit; processCount++) {
      if ((processID = fork()) < 0)
        DieWithError("fork() failed");
      else if (processID == 0) {  // child
        ProcessMain(servSock, ans);
        exit(EXIT_SUCCESS);
      }
    }
    multi_wait(processCount);
    exit(EXIT_SUCCESS);
  } else if (strcmp(ans, "N") == 0 || strcmp(ans, "n") == 0) {
    for (processCount = 0; processCount < processLimit; processCount++) {
      if ((processID = fork()) < 0)
        DieWithError("fork() failed");
      else if (processID == 0) {  // child
        selectAns(ans);
        printf("ans: %s\n", ans);
        ProcessMain(servSock, ans);
        exit(EXIT_SUCCESS);
      }
    }
    multi_wait(processCount);
    exit(EXIT_SUCCESS);
  }
}