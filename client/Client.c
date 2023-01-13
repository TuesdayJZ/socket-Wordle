// compile: gcc -w Client.c ClientLib.c -o Client
// run: ./Client <server IP> <port number>
#include "ClientLib.h"

int main(int argc, char *argv[]){
  int sock;                    /* Socket descriptor */
  struct sockaddr_in ServAddr; /* server address */
  unsigned short ServPort;     /* server port */
  char *servIP;                /* Server IP address (dotted quad) */

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <Server IP> <Port>\n", argv[0]);
    exit(1);
  }
  servIP = argv[1];         /* First arg: server IP address (dotted quad) */
  ServPort = atoi(argv[2]); /* Second arg: server port */

  /* Create a reliable, stream socket using TCP */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithError("socket() failed");

  /* Construct the server address structure */
  memset(&ServAddr, 0, sizeof(ServAddr));       /* Zero out structure */
  ServAddr.sin_family      = AF_INET;           /* Internet address family */
  ServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
  ServAddr.sin_port        = htons(ServPort);   /* Server port */

  /* Establish the connection to the server */
  printf("\n    trying to connect to the game server...\n");

  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
  // connect
  if (connect(sock, (struct sockaddr *)&ServAddr, sizeof(ServAddr)) < 0) {
    if (errno != EINPROGRESS)
      DieWithError("connect() failed");
    else {
      printf("    connection timed out.\n\n");
      exit(EXIT_SUCCESS);
    }
  }
  printf("    connected!\n\n");
  gameIntro(sock);
  gamePlay(sock);
  close(sock);
  exit(EXIT_SUCCESS);
}