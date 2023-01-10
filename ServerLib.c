#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdio.h>      /* for perror() */
#include <stdlib.h>     /* for exit() */
#include <string.h>     /* for memset() */
#include <sys/socket.h> /* for accept() */
#include <unistd.h>     /* for close() */
#include <time.h>
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

void gameMaster(int sock, char *ans) {
  int trial = 1;
  char recvBuffer[BUFSIZE], sendBuffer[BUFSIZE], copyAns[BUFSIZE];
  int recvMsgSize, sendMsgSize;
  if ((recvMsgSize = recv(sock, recvBuffer, BUFSIZE, 0)) < 0)
    DieWithError("recv() failed");
  int maxTrial = strtol(recvBuffer, NULL, 10);
  while (trial <= maxTrial) {
    printf("trial: %d\n", trial);
    if ((recvMsgSize = recv(sock, recvBuffer, BUFSIZE, 0)) < 0)
      DieWithError("recv() failed");
    if (recvMsgSize == 0) {
      printf("connection closed by client\n");
      exit(0);
    }
    printf("client try: %s\n", recvBuffer);

    // judge hit & blow
    strncpy(copyAns, ans, BUFSIZE);
    for (int i = 0; i < BUFSIZE; i++) sendBuffer[i] = '_';
    for (int i = 0; i < BUFSIZE; i++) {
      if (recvBuffer[i] == copyAns[i]) {
        sendBuffer[i] = 'H';
        copyAns[i] = 'D'; //done
      }
    }
    for (int i = 0; i < BUFSIZE; i++) {
      for (int j = 0; j < BUFSIZE; j++) {
        if (recvBuffer[i] == copyAns[j]) {
          sendBuffer[i] = 'B';
          copyAns[j] = 'D'; //done
        }
      }
    }

    // send hit & blow
    if(trial == maxTrial && strncmp(sendBuffer, "HHHHH", BUFSIZE) != 0){ // lose
      char copyBuffer[BUFSIZE];
      strncpy(copyBuffer, sendBuffer, BUFSIZE);
      strncpy(sendBuffer, "_LOSE", BUFSIZE);
      printf("sendBuffer: %s\n", sendBuffer);
      if (sendMsgSize = send(sock, sendBuffer, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      if (sendMsgSize = send(sock, copyBuffer, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      if (sendMsgSize = send(sock, ans, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      printf("GAME END\n");
      close(sock);
      exit(0);
    }else if(strncmp(sendBuffer, "HHHHH", BUFSIZE) == 0){ // win
      printf("sendBuffer: %s\n", sendBuffer);
      if (sendMsgSize = send(sock, sendBuffer, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      printf("GAME END\n");
      close(sock);
      exit(0);
    }else{
      printf("sendBuffer: %s\n", sendBuffer);
      if (sendMsgSize = send(sock, sendBuffer, BUFSIZE, 0) < 0)
        DieWithError("send() failed");
      trial++;
    }
  }
}

void selectAns(char *ans){
  // select answer word from wordlist file: ans.txt
  srand((unsigned)time(NULL));
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
  int clntSock;
  while (1) {
    printf("waiting connection...\n");
    if ((clntSock = AcceptConnection(servSock)) < 0)
      DieWithError("failed to accept connection");
    printf("with child process: %d\n", (unsigned int)getpid());
    gameMaster(clntSock, ans);
  }
}