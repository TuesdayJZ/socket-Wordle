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
  int processCount, processLimit; /* Number of processes */
  char wordle[BUFSIZE];          /* Wordle */
  char **words = (char **)malloc(sizeof(char *) * WORDS);
  int d;                       /* for multi player mode (players can play in different Wordle) */

  // argument check
  if (argc > 4 || argc < 2) {
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
      if (strcmp(argv[3], "-d") != 0) { // in multi player mode you can set option by adding "-d"
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
    printf("\n%d players with the same Wordle.\n", processLimit);
  } else if (argc > 3 && d == 1) {
    printf("\n%d players with different Wordles.\n", processLimit);
  }
  printf("The server is ready to accept connections.\n\n");

  readWords(words);

  // single player mode
  if (processLimit == 1) {
    selectWordle(wordle, words);
    ProcessMain(servSock, wordle, words);
    close(servSock);
    exit(EXIT_SUCCESS);
  }

  // multi player mode
  if (d == 0) { // players play with the same Wordle
    selectWordle(wordle, words);
    for (processCount = 0; processCount < processLimit; processCount++) {
      if ((processID = fork()) < 0)
        DieWithError("fork() failed");
      else if (processID == 0) { // child process
        ProcessMain(servSock, wordle, words);
        close(servSock);
        exit(EXIT_SUCCESS);
      }
    }
    multi_wait(processCount); // wait for all child processes
    exit(EXIT_SUCCESS);
  } else if (d == 1) { // players play with different Wordles
    for (processCount = 0; processCount < processLimit; processCount++) {
      if ((processID = fork()) < 0)
        DieWithError("fork() failed");
      else if (processID == 0) { // child process
        selectWordle(wordle, words);
        ProcessMain(servSock, wordle, words);
        close(servSock);
        exit(EXIT_SUCCESS);
      }
    }
    multi_wait(processCount); // wait for all child processes
    exit(EXIT_SUCCESS);
  }
}