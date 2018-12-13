//Client

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 5555
// Le port où le client va se connecter

#define MAXDATASIZE 1024
// Nombre maximum de bytes reçues

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in their_addr; // connector's address information

    if (argc != 7) {
        fprintf(stderr,"usage: synopsis erroné\n");
        return EXIT_FAILURE;
    }

    he=gethostbyname(argv[1]) ;
    if ( he == NULL) {  // get the host info
        perror("Client: gethostbyname");
        return EXIT_FAILURE;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0) ;
    if ( sockfd == -1) {
        perror("Client: socket");
        return EXIT_FAILURE;
    }

    their_addr.sin_family = AF_INET;    // host byte order
    their_addr.sin_port = htons(PORT);  // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("Client: connect");
        return EXIT_FAILURE;
    }

    int counter = 2 ;
    while (counter < argc ) {
      if (send(sockfd, argv[counter],strlen(argv[counter]),0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
      }
      if (counter == 2) {
        counter = 4 ;
      }
      else {
        counter++;
      }
      }

      numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0) ;
      if ( numbytes == -1) {
          perror("Client: recv");
          return EXIT_FAILURE;
      }

      buf[numbytes] = '\0';

      printf("Serveur envoie : %s",buf);

      close(sockfd);

      return EXIT_SUCCESS;
}
