#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdio.h>      /* for perror() */
#include <stdlib.h>     /* for exit() */
#include <string.h>     /* for memset() */
#include <sys/socket.h> /* for accept() */
#include <unistd.h>     /* for close() */
#include "ClientLib.h"

#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define BUFSIZE 5   /* Size of receive buffer */
#define WORDS 14855

void DieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(1);
}

int isAlp_toLower(char *str) {
  int cnt = 0;
  for (int i = 0; i < BUFSIZE; i++) {
    if (str[i] >= 'A' && str[i] <= 'Z') {
      str[i] += 32;
    }else if (str[i] >= 'a' && str[i] <= 'z') {
      cnt += 0;
    }else{
      cnt++;
    }
  }
  return cnt;
}

void readAllList(char **allList){
  FILE *fp;
  char buf[6];
  int i = 0;
  if ((fp = fopen("words.txt", "r")) == NULL)
    DieWithError("fopen failed");
  for (int i = 0; i < WORDS; i++){
    fseek(fp, (BUFSIZE + 1) * i, SEEK_SET);
    fgets(buf, BUFSIZE + 1, fp);
    allList[i] = (char *)malloc(sizeof(char) * BUFSIZE);
    strncpy(allList[i], buf, BUFSIZE);
  }
  fclose(fp);
}

void gamePlay(int sock) {
  int trial = 1;
  char sendBuffer[BUFSIZE], recvBuffer[BUFSIZE];
  int AnsSize, sendMsgSize, recvMsgSize;
  int hit = 0;
  char **allList = (char **)malloc(sizeof(char *) * WORDS);
  readAllList(allList);

  while (1) {
    printf("\n    ==========================================\n\n");
    printf("        [trial %d] Enter a word: ", trial);
    scanf("%s", sendBuffer);
    AnsSize = strlen(sendBuffer);
    trial++;
    if(strncmp(sendBuffer, "quit", 4) == 0){
      printf("\n        You quitted the game.\n\n");
      close(sock);
      exit(0);
    } else if (isAlp_toLower(sendBuffer) != 0) {
      printf("\n        Please enter ALPHABETS.\n");
      trial--;
      continue;
    } else if (AnsSize != 5) {
      printf("\n        Please enter a 5 letter word.\n");
      trial--;
      continue;
    } else {
      // check if word is in list
      for (int i = 0; i < WORDS; i++) {
        if (strncmp(sendBuffer, allList[i], 5) == 0) {
          hit = 1;
          break;
        }
      }
      if (hit == 0) {
        printf("\n        Please enter a word in the word list.\n");
        trial--;
        continue;
      }
      hit = 0;
    }
    if (sendMsgSize = send(sock, sendBuffer, AnsSize, 0) < 0)
      DieWithError("send() failed");
    if (recvMsgSize = recv(sock, recvBuffer, 5, 0) < 0)
      DieWithError("recv() failed");
    if(strncmp(recvBuffer, "_LOSE", 5) == 0){
      if (recvMsgSize = recv(sock, recvBuffer, 5, 0) < 0)
        DieWithError("recv() failed");
      printf("                              [ %s ]\n", recvBuffer);
      printf("\n    ==========================================\n");
      printf("\n        YOU LOSE :(\n\n");
      if (recvMsgSize = recv(sock, recvBuffer, 5, 0) < 0)
        DieWithError("recv() failed");
      printf("        The Wordle is \"%s\".\n\n", recvBuffer);
      close(sock);
      exit(0);
    }
    for (int i = 0; i < 5; i++) {
      if (recvBuffer[i] == 'H') {
        hit++;
        if (hit == 5) {
          printf("                              [ %s ]\n", recvBuffer);
          printf("\n    ==========================================\n");
          printf("\n        YOU WIN in %d try :)\n\n", trial - 1);
          close(sock);
          exit(0);
        }
      }
    }
    printf("                              [ %s ]\n", recvBuffer);
    hit = 0;
  }
}

void gameIntro(int sock){
  char ans[10];
  while (1) {
    printf("    Do you want to play a game? (y/n): ");
    scanf("%c", &ans);
    if (ans[0] == 'y' || ans[0] == 'Y') {
      printf("\n    ==========================================\n");
      printf("    =                 WORDLE                 =\n");
      printf("    ==========================================\n\n");
      printf("    Guess the Wordle in limited tries.\n");
      printf("\n    -----------------------------------------\n");
      printf("    Each guess must be a 5 letter word.\n");
      printf("    \"H\", \"B\", \"_\" will be shown for each letter.\n");
      printf("    \"H\" means the letter is in the right place.\n");
      printf("    \"B\" means the letter is in the Wordle.\n");
      printf("    \"_\" means the letter is not in the Wordle.\n\n");
      printf("    You can quit the game by entering \"quit\".\n");
      printf("    -----------------------------------------\n\n");
      printf("    You can select how many tries you want (enter a number) : ");
      while(1){
        scanf("%s", &ans);
        if (strtoll(ans, NULL, 10) == 0) {
          printf("    Please input a number : ");
          continue;
        }
        break;
      }
      if (send(sock, ans, 1, 0) < 0)
        DieWithError("send() failed");
      printf("\n    OK, you can try %s times. \n    let's start the game!\n", ans);
      break;
    } else if (ans[0] == 'n' || ans[0] == 'N') {
      printf("    OK, bye.\n");
      close(sock);
      exit(0);
    } else {
      printf("    please input y or n.\n");
      continue;
    }
  }
}