// BAKKALI Yahya
// 00445166
// Client Code

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define PORT 5555
// Le port du serveur où le client va se connecter
#define MAXDATASIZE 1024
// Nombre maximum des informations

int main(int argc, char *argv[])
{
    if (argc != 7) {
        fprintf(stderr,"Error : synopsis\n");
        return EXIT_FAILURE;
    }

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
    int usernameSize = strlen(argv[2]) ;
    int filenameSize = strlen(filename) ;
/////////
    if (send(sockfd, &usernameSize ,sizeof(int),0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send size\n") ;

    if (send(sockfd, &filenameSize  ,sizeof(int),0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send size\n") ;

//////////
    if (send(sockfd,argv[2],usernameSize,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send \n") ;

    if (send(sockfd,argv[4],4,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send \n") ;

    if (send(sockfd,argv[5],2,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send \n") ;

    if (send(sockfd,argv[6],2,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send \n") ;

    if (send(sockfd,filename,filenameSize,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send \n") ;
////////

    FILE *fptr = fopen(argv[3], "r");
    // Fichier a envoyer
    if ( fptr == NULL ) {
       perror("Error : No such file");
       return EXIT_FAILURE;
    }
    fseek(fptr, 0 , SEEK_END);
    int fileSize = ftell(fptr);
    rewind(fptr);

    if (send(sockfd, &fileSize , sizeof(int) ,0)==-1) {
      perror("Client: send error");
      return EXIT_FAILURE;
    }
/////////
    char buffer[MAXDATASIZE] ;
    int numbytes = 1;
    while ((numbytes = fread(buffer,sizeof(char),MAXDATASIZE,fptr))) {
        if (send(sockfd,buffer,numbytes,0)==-1) {
            perror("Client: send error");
            return EXIT_FAILURE;
        }
    }

/////////

    char EXIT_Message[MAXDATASIZE+1] ;
    if ( recv(sockfd, EXIT_Message ,MAXDATASIZE, 0) == -1) {
        // Reception du code 0 en cas de succès sinon 1 et l'erreur parvenue
        perror("Client: recv");
        return EXIT_FAILURE;
    }

////////

    char exitStat = EXIT_Message[0] ;
    printf("Server send code %c\n",exitStat );
    if ( exitStat != '0' ) {
      // Une erreur est reçue du côte serveur
      fprintf(stderr,"%s\n",EXIT_Message+1);
      return EXIT_FAILURE;
    }
    close(sockfd);
    // Fermer le socket
    fclose(fptr) ;
    // Fermer le fichier photo
    return EXIT_SUCCESS;
    }
