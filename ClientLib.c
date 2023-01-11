#include "ClientLib.h"

#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define BUFSIZE 5   /* Size of receive buffer */
#define WORDS 14855

void DieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(EXIT_FAILURE);
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

void readWords(char **words){
  FILE *fp;
  char buf[6];
  int i = 0;
  if ((fp = fopen("words.txt", "r")) == NULL)
    DieWithError("fopen failed");
  for (int i = 0; i < WORDS; i++){
    fseek(fp, (BUFSIZE + 1) * i, SEEK_SET);
    fgets(buf, BUFSIZE + 1, fp);
    words[i] = (char *)malloc(sizeof(char) * BUFSIZE);
    strncpy(words[i], buf, BUFSIZE);
  }
  fclose(fp);
}

void gamePlay(int sock) {
  int trial = 1;
  char sendBuffer[BUFSIZE], recvBuffer[BUFSIZE];
  int AnsSize, sendMsgSize, recvMsgSize;
  int hit = 0;
  char **words = (char **)malloc(sizeof(char *) * WORDS);
  readWords(words);
  while (1) {
    printf("\n    ==========================================\n\n");
    printf("        [trial %d] Enter a word: ", trial);
    scanf("%s", sendBuffer);
    AnsSize = strlen(sendBuffer);
    trial++;
    if(strncmp(sendBuffer, "quit", 4) == 0){
      if ((sendBuffer[4]<= 'z' && sendBuffer[4] >= 'a') || (sendBuffer[4] <= 'Z' && sendBuffer[4] >= 'A')) {
        ;
      } else {
        printf("\n        You quitted the game.\n\n");
        strncpy(sendBuffer, "\QUIT", 5);
        if (sendMsgSize = send(sock, sendBuffer, AnsSize, 0) < 0)
          DieWithError("send() failed");
        exit(EXIT_SUCCESS);
      }
    } else if (isAlp_toLower(sendBuffer) != 0 || AnsSize != BUFSIZE) {
      printf("\n        Please enter 5 ALPHABETs.\n");
      trial--;
      continue;
    } else {
      // check if word is in list
      for (int i = 0; i < WORDS; i++) {
        if (strncmp(sendBuffer, words[i], 5) == 0) {
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
      exit(EXIT_SUCCESS);
    }
    for (int i = 0; i < 5; i++) {
      if (recvBuffer[i] == 'H') {
        hit++;
        if (hit == 5) {
          printf("                              [ %s ]\n", recvBuffer);
          printf("\n    ==========================================\n");
          printf("\n        YOU WIN in %d try :)\n\n", trial - 1);
          close(sock);
          exit(EXIT_SUCCESS);
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
        if (strncmp(ans, "quit", 4) == 0) {
          printf("\n        You quitted the game.\n\n");
          exit(EXIT_SUCCESS);
        }
        else if (strtoll(ans, NULL, 10) > 9 || strtoll(ans, NULL, 10) < 1) {
          printf("    Please input a number (1 to 9) : ");
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
      exit(EXIT_SUCCESS);
    } else {
      printf("    please input y or n.\n");
      continue;
    }
  }
}