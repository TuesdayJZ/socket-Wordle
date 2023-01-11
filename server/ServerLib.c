#include "ServerLib.h"

void DieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(1);
}

int CreateServerSocket(unsigned short port) {
  int sock;                    /* Socket descriptor for server */
  struct sockaddr_in ServAddr; /* Local address */
  char ipstr[INET_ADDRSTRLEN];

  /* Create socket for incoming connections */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithError("socket() failed");

  /* Construct local address structure */
  memset(&ServAddr, 0, sizeof(ServAddr));       /* Zero out structure */
  ServAddr.sin_family = AF_INET;                /* Internet address family */
  ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
  ServAddr.sin_port = htons(port);              /* Local port */

  /* Bind to the local address */
  if (bind(sock, (struct sockaddr *)&ServAddr, sizeof(ServAddr)) < 0)
    DieWithError("bind() failed");

  /* print IP for convenience */
  printf("This Server's IP address: %s\n", inet_ntop(ServAddr.sin_family, &ServAddr.sin_addr, ipstr, sizeof(ipstr)));

  /* Mark the socket so it will listen for incoming connections */
  if (listen(sock, MAXPENDING) < 0) DieWithError("listen() failed");
  return sock;
}

int AcceptConnection(int servSock) {
  int clntSock;                /* Socket descriptor for client */
  struct sockaddr_in ClntAddr; /* Client address */
  unsigned int clntLen;        /* Length of client address data structure */

  /* Set the size of the in-out parameter */
  clntLen = sizeof(ClntAddr);

  /* Wait for a client to connect */
  if ((clntSock = accept(servSock, (struct sockaddr *)&ClntAddr, &clntLen)) < 0)
    DieWithError("accept() failed");

  printf("Handling client %s\n", inet_ntoa(ClntAddr.sin_addr));
  return clntSock;
}

void gameMaster(int sock, char *wordle, int pid, char **words) {
  int trial = 1;
  char recvBuffer[BUFSIZE], sendBuffer[BUFSIZE], copywordle[BUFSIZE];
  int recvMsgSize, sendMsgSize;

  // receive max trial
  if ((recvMsgSize = recv(sock, recvBuffer, BUFSIZE, 0)) < 0)
    DieWithError("recv() failed");
  int maxTrial = strtol(recvBuffer, NULL, 10);

  // game loop
  while (trial <= maxTrial) {
    // receive word from client
    if ((recvMsgSize = recv(sock, recvBuffer, BUFSIZE, 0)) < 0)
      DieWithError("recv() failed");
    if (recvMsgSize == 0) {
      printf("connection closed by client\n");
      exit(EXIT_SUCCESS);
    }

    // client entered "quit"
    if (strncmp(recvBuffer, "\QUIT", 5) == 0) {
      printf("GAME END (QUIT)\n");
      close(sock);
      exit(EXIT_SUCCESS);
    }

    // print received word
    printf("[%d] (trial %d) received: %.5s\n", pid,trial, recvBuffer);

    // check if it's a word
    for (int i = 0; i < WORDS; i++){
      if (strncmp(recvBuffer, words[i], BUFSIZE) == 0) {
        strncpy(sendBuffer, "11111", BUFSIZE); // find it in the list
        break;
      }
      strncpy(sendBuffer, "00000", BUFSIZE);
    }

    if(strncmp(sendBuffer, "11111", BUFSIZE) == 0){
      // judge hit & blow
      strncpy(copywordle, wordle, BUFSIZE);
    for (int i = 0; i < BUFSIZE; i++) sendBuffer[i] = '_';
    for (int i = 0; i < BUFSIZE; i++) {
      if (recvBuffer[i] == copywordle[i]) {
        sendBuffer[i] = '#'; // hit
        copywordle[i] = '.'; // this letter done
      }
    }
    for (int i = 0; i < BUFSIZE; i++) {
      for (int j = 0; j < BUFSIZE; j++) {
        if (recvBuffer[i] == copywordle[j]) {
          sendBuffer[i] = '/'; // blow
          copywordle[j] = '.'; // this letter done
        }
      }
    }
    }

    // send "00000" if it's not a word
    if (strncmp(sendBuffer, "00000", BUFSIZE) == 0) {
      if (sendMsgSize = send(sock, sendBuffer, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      continue; // try again
    }

    // if it's a word, send hit & blow
    else if (trial == maxTrial &&
             strncmp(sendBuffer, "#####", BUFSIZE) != 0) { // lose
      char copyBuffer[BUFSIZE];
      strncpy(copyBuffer, sendBuffer, BUFSIZE); // save hit & blow
      strncpy(sendBuffer, "_LOSE", BUFSIZE); // send "_LOSE"
      if (sendMsgSize = send(sock, sendBuffer, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      if (sendMsgSize = send(sock, copyBuffer, BUFSIZE, 0) < 0) // send hit & blow
        DieWithError("send() failed");
      if (sendMsgSize = send(sock, wordle, BUFSIZE, 0) < 0) // send wordle
        DieWithError("send() failed");
      printf("[%d] GAME END (LOSE)\n", pid);
      close(sock);
      exit(EXIT_SUCCESS);
    } else if (strncmp(sendBuffer, "#####", BUFSIZE) == 0) {  // win
      if (sendMsgSize = send(sock, sendBuffer, BUFSIZE, 0) < 0) // send hit & blow
        DieWithError("send() failed");
      printf("[%d] GAME END (WIN)\n", pid);
      close(sock);
      exit(EXIT_SUCCESS);
    } else { // continue game
      if (sendMsgSize = send(sock, sendBuffer, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      trial++;
    }
  }
}

void selectWordle(char *wordle, char **words) {
  // select wordle randomly
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_sec + tv.tv_usec);
  int no = rand() % WORDS;
  strncpy(wordle, words[no], BUFSIZE);
}

void readWords(char **words) {
  // read words from words.txt, and store them in words[]
  FILE *fp;
  char buf[6];
  int i = 0;
  if ((fp = fopen("words.txt", "r")) == NULL) exit(1);
  for (int i = 0; i < WORDS; i++) {
    fseek(fp, (BUFSIZE + 1) * i, SEEK_SET);
    fgets(buf, BUFSIZE + 1, fp);
    words[i] = (char *)malloc(sizeof(char) * BUFSIZE);
    strncpy(words[i], buf, BUFSIZE);
  }
  fclose(fp);
}

void ProcessMain(int servSock, char *wordle, char **words) {
  // each child process runs this
  int clntSock, pid;
  while (1) {
    printf("waiting connection ...\n");
    if ((clntSock = AcceptConnection(servSock)) < 0)
      DieWithError("failed to accept connection");
    pid = getpid();
    printf("[%d] accepted.\n", pid);
    printf("[%d] Wordle : %s\n", pid, wordle);
    gameMaster(clntSock, wordle, pid, words);
  }
}

void multi_wait(int processCount){
  // wait for all child processes
  while(1){
    pid_t pid;
    int status = 0;
    pid = wait(&status);
    if (pid < 0) {
      if (errno == ECHILD) break;
      else if (errno == EINTR)
        continue;
      DieWithError("wait error");
    }
    --processCount;
    printf("[%d] terminated.\n\n", pid);
    if (processCount == 0) printf("All process terminated.\n");
  }
}