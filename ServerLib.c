#include "ServerLib.h"

#define MAXPENDING 5  /* Maximum outstanding connection requests */
#define BUFSIZE 5 /* Size of receive buffer */
#define W0RDS 14855

void DieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(1);
}

int CreateServerSocket(unsigned short port) {
  int sock;                        /* socket to create */
  struct sockaddr_in ServAddr; /* Local address */

  /* Create socket for incoming connections */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithError("socket() failed");

  /* Construct local address structure */
  memset(&ServAddr, 0, sizeof(ServAddr)); /* Zero out structure */
  ServAddr.sin_family = AF_INET;              /* Internet address family */
  ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
  ServAddr.sin_port = htons(port);              /* Local port */

  /* Bind to the local address */
  if (bind(sock, (struct sockaddr *)&ServAddr, sizeof(ServAddr)) < 0)
    DieWithError("bind() failed");

  printf("This Server's IP address: %s\n", inet_ntoa(ServAddr.sin_addr));

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

void gameMaster(int sock, char *ans, int pid) {
  int trial = 1;
  char recvBuffer[BUFSIZE], sendBuffer[BUFSIZE], copyAns[BUFSIZE];
  int recvMsgSize, sendMsgSize;
  if ((recvMsgSize = recv(sock, recvBuffer, BUFSIZE, 0)) < 0)
    DieWithError("recv() failed");
  int maxTrial = strtol(recvBuffer, NULL, 10);
  while (trial <= maxTrial) {
    if ((recvMsgSize = recv(sock, recvBuffer, BUFSIZE, 0)) < 0)
      DieWithError("recv() failed");
    if (recvMsgSize == 0) {
      printf("connection closed by client\n");
      exit(EXIT_SUCCESS);
    }
    if (strncmp(recvBuffer, "\QUIT", 5) == 0) {
      printf("GAME END (QUIT)\n");
      close(sock);
      exit(EXIT_SUCCESS);
    }
    printf("[%d] (trial %d) received: %.5s\n", pid,trial, recvBuffer);
    // judge hit & blow
    strncpy(copyAns, ans, BUFSIZE);
    for (int i = 0; i < BUFSIZE; i++) sendBuffer[i] = '_';
    for (int i = 0; i < BUFSIZE; i++) {
      if (recvBuffer[i] == copyAns[i]) {
        sendBuffer[i] = '#';
        copyAns[i] = 'D'; //done
      }
    }
    for (int i = 0; i < BUFSIZE; i++) {
      for (int j = 0; j < BUFSIZE; j++) {
        if (recvBuffer[i] == copyAns[j]) {
          sendBuffer[i] = '/';
          copyAns[j] = 'D'; //done
        }
      }
    }

    // send hit & blow
    if(trial == maxTrial && strncmp(sendBuffer, "#####", BUFSIZE) != 0){ // lose
      char copyBuffer[BUFSIZE];
      strncpy(copyBuffer, sendBuffer, BUFSIZE);
      strncpy(sendBuffer, "_LOSE", BUFSIZE);
      if (sendMsgSize = send(sock, sendBuffer, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      if (sendMsgSize = send(sock, copyBuffer, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      if (sendMsgSize = send(sock, ans, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      printf("[%d] GAME END (LOSE)\n", pid);
      close(sock);
      exit(EXIT_SUCCESS);
    }else if(strncmp(sendBuffer, "#####", BUFSIZE) == 0){ // win
      if (sendMsgSize = send(sock, sendBuffer, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      printf("[%d] GAME END (WIN)\n", pid);
      close(sock);
      exit(EXIT_SUCCESS);
    }else{
      if (sendMsgSize = send(sock, sendBuffer, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      trial++;
    }
  }
}

void selectAns(char *ans){
  // select answer word from wordlist file: ans.txt
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_sec + tv.tv_usec);
  FILE *fp;
  char buff[BUFSIZE + 1];
  int no = rand() % W0RDS + 1;
  fp = fopen("words.txt", "r");
  if (!fp)
    DieWithError("fopen failed");
  fseek(fp, no * (BUFSIZE + 1), SEEK_SET);
  fgets(buff, BUFSIZE + 1, fp);
  strncpy(ans, buff, BUFSIZE);
  fclose(fp);
}

void ProcessMain(int servSock, char *ans) {
  int clntSock, pid;
  while (1) {
    printf("waiting connection ...\n");
    if ((clntSock = AcceptConnection(servSock)) < 0)
      DieWithError("failed to accept connection");
    pid = getpid();
    printf("[%d] accepted.\n", pid);
    gameMaster(clntSock, ans, pid);
  }
}

void multi_wait(int processCount){
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