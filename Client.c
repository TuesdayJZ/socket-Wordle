// compile: gcc -w Client.c ClientLib.c -o Client
// run: ./Client <server IP> <port number>
#include "ClientLib.h"

int main(int argc, char *argv[]){
  int sock;                        /* Socket descriptor */
  struct sockaddr_in ServAddr; /* server address */
  unsigned short ServPort;     /* server port */
  char *servIP;                    /* Server IP address (dotted quad) */

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <Server IP> <Port>\n", argv[0]);
    exit(1);
  }
  servIP = argv[1];             /* First arg: server IP address (dotted quad) */
  ServPort = atoi(argv[2]); /* Second arg: server port */

  /* Create a reliable, stream socket using TCP */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithError("socket() failed");

  /* Construct the server address structure */
  memset(&ServAddr, 0, sizeof(ServAddr));     /* Zero out structure */
  ServAddr.sin_family      = AF_INET;             /* Internet address family */
  ServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
  ServAddr.sin_port        = htons(ServPort); /* Server port */

  /* Establish the connection to the server */
  printf("    trying to connect to the game server...\n");
  if (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
    DieWithError("connect() failed");
  printf("    connected!\n");
  gameIntro(sock);
  gamePlay(sock);
  close(sock);
  exit(EXIT_SUCCESS);
}