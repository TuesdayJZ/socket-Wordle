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

  // set socket to non-blocking
  int flag = fcntl(sock, F_GETFL);
  if (flag < 0)
    DieWithError("fcntl() failed");
  if (fcntl(sock, F_SETFL, flag | O_NONBLOCK) < 0)
    DieWithError("fcntl() failed");

  // connect
  int connectresult = connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr));
  if (connectresult < 0) { // connect failed
    if (errno == EINPROGRESS) errno = 0; // ignore EINPROGRESS
    else{
      fcntl(sock, F_SETFL, flag);
      DieWithError("connect() failed");
    }
  }

  // set socket back to blocking
  if(fcntl(sock, F_SETFL, flag) < 0)
    DieWithError("fcntl() failed");

  // select to check if socket is ready
  fd_set readfd, writefd;
  FD_ZERO(&readfd);
  FD_ZERO(&writefd);
  FD_SET(sock, &readfd);
  FD_SET(sock, &writefd);

  // timeout : 5 sec
  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;
  int selectresult = select(sock + 1, &readfd, &writefd, NULL, &timeout);
  if (selectresult == 0) // timeout
    DieWithError("select() failed... timeout");
  else if(!FD_ISSET(sock, &readfd) && !FD_ISSET(sock, &writefd)) /* socket can't read and write */
    DieWithError("select() failed");
  printf("    connected!\n\n");
  gameIntro(sock);
  gamePlay(sock);
  close(sock);
  exit(EXIT_SUCCESS);
}