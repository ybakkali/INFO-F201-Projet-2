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
    if (argc != 7) {
        fprintf(stderr,"Error : synopsis\n");
        return EXIT_FAILURE;
    }

    char buffer[MAXDATASIZE];

    struct hostent *he;
    struct sockaddr_in their_addr; // connector's address information

    he=gethostbyname(argv[1]) ;
    if ( he == NULL) {  // get the host info
        perror("Client: gethostbyname");
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0) ;
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

    char *filename = strrchr(argv[3],'/')+1;
    int index[4] = {2,4,5,6} ;
    char *message ;
    int counter ;
    for (counter = 0 ; counter < 5 ; ++counter ) {
      switch (counter) {
        case 4 :
            message = filename ;
            break ;
        default :
            message = argv[index[counter]] ;
      }
      if (send(sockfd,message,MAXDATASIZE,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
      }
    }

    int numbytes = recv(sockfd, buffer, MAXDATASIZE, 0) ;
    if ( numbytes == -1) {
        perror("Client: recv");
        return EXIT_FAILURE;
    }
    buffer[numbytes] = '\0';
    printf("Server send :\n%s",buffer);

    FILE *fptr = fopen(argv[3], "r");
    if ( fptr == NULL ) {
       perror("Error : No such file");
       return EXIT_FAILURE;
    }
    fseek(fptr, 0 , SEEK_END);
    unsigned long fileSize = ftell(fptr);
    rewind(fptr);
    printf("%lu \n",fileSize) ;

    if (send(sockfd, &fileSize , sizeof(long) ,0)==-1) {
      perror("Client: send error");
      return EXIT_FAILURE;
    }

    int nb = 1;
    while ((nb = fread(buffer,sizeof(char),MAXDATASIZE,fptr))) {
      if (send(sockfd,buffer,nb,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
      }
    }
    close(sockfd);
    fclose(fptr) ;
    return EXIT_SUCCESS;
}
