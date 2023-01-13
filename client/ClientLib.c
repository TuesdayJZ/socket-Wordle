#include "ClientLib.h"

void DieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(EXIT_FAILURE);
}

int isAlp_toLower(char *str) {
  int cnt = 0;
  for (int i = 0; i < BUFSIZE; i++) {
    if (str[i] >= 'A' && str[i] <= 'Z') {
      str[i] += 32;
    } else if (str[i] >= 'a' && str[i] <= 'z') {
      cnt += 0;
    } else {
      cnt++;
    }
  }
  return cnt;
}

void gamePlay(int sock) {
  int trial = 1;
  char sendBuffer[BUFSIZE], recvBuffer[BUFSIZE];
  int AnsSize, sendMsgSize, recvMsgSize;
  int hit = 0;
  while (1) {
    printf("\n    ==========================================\n\n");
    printf("        [trial %d] Enter a word: ", trial);
    scanf("%s", sendBuffer);
    AnsSize = strlen(sendBuffer);
    trial++;
    if (strncmp(sendBuffer, "quit", 4) == 0) {
      if ((sendBuffer[4] <= 'z' && sendBuffer[4] >= 'a') ||
          (sendBuffer[4] <= 'Z' && sendBuffer[4] >= 'A')) {
        ;
      } else {
        printf("\n        You quitted the game.\n\n");
        strncpy(sendBuffer, "\QUIT", 5);
        if (sendMsgSize = send(sock, sendBuffer, BUFSIZE, 0) < 0)
          DieWithError("send() failed");
        close(sock);
        exit(EXIT_SUCCESS);
      }
    } else if (isAlp_toLower(sendBuffer) != 0 || AnsSize != BUFSIZE) {
      printf("\n        Please enter 5 ALPHABETs.\n");
      trial--;
      continue;
    }
    if (sendMsgSize = send(sock, sendBuffer, AnsSize, 0) < 0)
      DieWithError("send() failed");
    if (recvMsgSize = recv(sock, recvBuffer, 5, 0) < 0)
      DieWithError("recv() failed");
    if (strncmp(recvBuffer, "00000", 5) == 0) {
      printf("\n        It's NOT a WORD, try again.\n");
      trial--;
      continue;
    }
    if (strncmp(recvBuffer, "_LOSE", 5) == 0) {
      if (recvMsgSize = recv(sock, recvBuffer, 5, 0) < 0)
        DieWithError("recv() failed");
      printf("\n                              [ %s ]\n", recvBuffer);
      printf("\n    ==========================================\n");
      printf("\n        YOU LOSE :(\n\n");
      if (recvMsgSize = recv(sock, recvBuffer, 5, 0) < 0)
        DieWithError("recv() failed");
      printf("        The Wordle is \"%s\".\n\n", recvBuffer);
      close(sock);
      exit(EXIT_SUCCESS);
    }
    for (int i = 0; i < 5; i++) {
      if (recvBuffer[i] == '#') {
        hit++;
        if (hit == 5) {
          printf("\n                              [ %s ]\n", recvBuffer);
          printf("\n    ==========================================\n");
          printf("\n        YOU WIN in %d try :)\n\n", trial - 1);
          printf("        The Wordle is \"%.5s\".\n\n", sendBuffer);
          close(sock);
          exit(EXIT_SUCCESS);
        }
      }
    }
    printf("\n                              [ %s ]\n", recvBuffer);
    hit = 0;
  }
}

void gameIntro(int sock) {
  char ans[10];
  printf("    Do you want to play a game? (y/n): ");
  while (1) {
    scanf("%s", &ans);
    if (strcmp(ans, "y") == 0 || strcmp(ans, "Y") == 0 ||
        strcmp(ans, "n") == 0 || strcmp(ans, "N") == 0)
      break;
    else
      printf("    Please enter Y/n : ");
  }
  if (strcmp(ans, "y") == 0 || strcmp(ans, "Y") == 0) {
    printf("\n    ==========================================\n");
    printf("    =                 WORDLE                 =\n");
    printf("    ==========================================\n\n");
    printf("    Guess the Wordle in limited tries.\n");
    printf("\n    -----------------------------------------\n\n");
    printf("    Each guess must be a 5 letter word.\n\n");
    printf("    #, /, _ will be shown for each letter.\n\n");
    printf("    # means the letter is in the right place.\n");
    printf("    / means the letter is in the Wordle.\n");
    printf("    _ means the letter is not in the Wordle.\n\n");
    printf("    You can quit the game by entering \"quit\".\n\n");
    printf("    -----------------------------------------\n\n");
    printf("    How many trials? : ");
    while (1) {
      scanf("%s", &ans);
      if (strncmp(ans, "quit", 4) == 0) {
        printf("\n        You quitted the game.\n\n");
        close(sock);
        exit(EXIT_SUCCESS);
      } else if (strtoll(ans, NULL, 10) > 9 || strtoll(ans, NULL, 10) < 1) {
        printf("    Please input a number (1 to 9) : ");
        continue;
      }
      break;
    }
    if (send(sock, ans, 1, 0) < 0) DieWithError("send() failed");
    printf("\n    OK, you can try %s times. \n    let's start the game!\n",
           ans);
  } else if (strcmp(ans, "n") == 0 || strcmp(ans, "N") == 0) {
    printf("    OK, bye.\n");
    close(sock);
    exit(EXIT_SUCCESS);
  }
}