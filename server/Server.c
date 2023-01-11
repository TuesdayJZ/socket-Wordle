// compile: gcc -w Server.c ServerLib.c -o Server
// run: ./Server <port number> (<number of players> <-s or -d>)
#include "ServerLib.h"

#define BUFSIZE 5
#define MAXPLAYERS 10
#define WORDS 14855

int main(int argc, char *argv[]) {
  int servSock; /* Socket descriptor for server */
  int clntSock; /* Socket descriptor for client */
  pid_t processID;
  int processCount, processLimit;
  char wordle[BUFSIZE];
  char **words = (char **)malloc(sizeof(char *) * WORDS);
  int d;

  if (argc > 4 || argc < 2) /* Test for correct number of arguments */ {
    fprintf(stderr, "Usage: %s <Server Port> (<Number of Players> <-d>)\n",
            argv[0]);
    exit(1);
  }

  if (strtol(argv[1], NULL, 10) < 1) {
    fprintf(stderr, "Server Port should be a number.\n");
    exit(1);
  }

  unsigned short ServPort = atoi(argv[1]); /* First arg: local port */
  if (argc == 2)
    processLimit = 1;
  else {
    d = 0;
    if (strtol(argv[2], NULL, 10) < 1 ||
        strtol(argv[2], NULL, 10) > MAXPLAYERS) {
      fprintf(stderr, "Number of players should be posotive number below %d.\n",
              MAXPLAYERS);
      exit(1);
    }
    if (argc == 4) {
      if (strcmp(argv[3], "-d") != 0) {
        fprintf(stderr,
                "Usage: %s <Server Port> (<Number of Players> <-d>)\nYou can "
                "play with different Wordle by -d.\n",
                argv[0]);
        exit(1);
      } else {
        d = 1;
      }
    }
    processLimit = atoi(argv[2]);
  }

  if ((servSock = CreateServerSocket(ServPort)) < 0)
    DieWithError("failed to create server socket");

  if (argc > 3 && d == 0) {
    printf("\nYou can play %d players with same Wordle.\n", processLimit);
  } else if (argc > 3 && d == 1) {
    printf("\nYou can play %d players with different Wordle.\n", processLimit);
  }
  printf("The server is ready to accept connections.\n\n");

  readWords(words);

  if (processLimit == 1) {
    selectWordle(wordle, words);
    printf("Wordle : %s\n\n", wordle);
    ProcessMain(servSock, wordle);
    close(servSock);
    exit(EXIT_SUCCESS);
  }

  if (d == 0) {
    selectWordle(wordle, words);
    printf("Wordle : %s\n\n", wordle);
    for (processCount = 0; processCount < processLimit; processCount++) {
      if ((processID = fork()) < 0)
        DieWithError("fork() failed");
      else if (processID == 0) {  // child
        ProcessMain(servSock, wordle);
        close(servSock);
        exit(EXIT_SUCCESS);
      }
    }
    multi_wait(processCount); // wait for all child processes
    exit(EXIT_SUCCESS);
  } else if (d == 1) {
    for (processCount = 0; processCount < processLimit; processCount++) {
      if ((processID = fork()) < 0)
        DieWithError("fork() failed");
      else if (processID == 0) {  // child
        selectWordle(wordle, words);
        ProcessMain(servSock, wordle);
        close(servSock);
        exit(EXIT_SUCCESS);
      }
    }
    multi_wait(processCount); // wait for all child processes
    exit(EXIT_SUCCESS);
  }
}