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
#define h_addr h_addr_list[0]

int main(int argc, char *argv[]) {

    if (argc != 7) {
        fprintf(stderr,"Error : synopsis\n");
        return EXIT_FAILURE;
    }

    struct hostent *he;
    struct sockaddr_in their_addr;
    // Informations sur l'adresse du connecteur

    he=gethostbyname(argv[1]) ;
    // Obtenir les infos sur l'hôte
    if ( he == NULL) {
        perror("Client: gethostbyname");
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0) ;
    if ( sockfd == -1) {
        perror("Client: socket");
        return EXIT_FAILURE;
    }

    their_addr.sin_family = AF_INET;
    // ordre des octets de l'hôte
    their_addr.sin_port = htons(PORT);
    // court, ordre d'octets réseau
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);
    // zéro pour le reste de la structure

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("Client: connect");
        return EXIT_FAILURE;
    }

    char *filename = strrchr(argv[3],'/')+1;
    int usernameSize = strlen(argv[2]) ;
    int filenameSize = strlen(filename) ;

    /* Envoie de la taille d'utilisateur et du nom de fichier */

    if (send(sockfd, &usernameSize ,sizeof(int),0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send username size : %d\n",usernameSize) ;

    if (send(sockfd, &filenameSize  ,sizeof(int),0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send filename size : %d\n",filenameSize) ;

    /* Envoie du nom d'utilisateur,année,mois,jour,nom du ficher */

    if (send(sockfd,argv[2],usernameSize,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send username : %s\n",argv[2]) ;

    if (send(sockfd,argv[4],4,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send year : %s\n",argv[4]) ;

    if (send(sockfd,argv[5],2,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send month : %s\n",argv[5]) ;

    if (send(sockfd,argv[6],2,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send day : %s\n",argv[6]) ;

    if (send(sockfd,filename,filenameSize,0)==-1) {
        perror("Client: send error");
        return EXIT_FAILURE;
    }

    printf("Send filename : %s\n",filename) ;

    /* Envoie de la taille du fichier et du fichier */

    FILE *fptr = fopen(argv[3], "r");
    // Ouvrir le fichier à envoyer
    if ( fptr == NULL ) {
       perror("Error : Open file return NULL");
       return EXIT_FAILURE;
    }
    fseek(fptr, 0 , SEEK_END);
    int fileSize = ftell(fptr);
    rewind(fptr);

    if (send(sockfd, &fileSize , sizeof(int) ,0)==-1) {
      perror("Client: send error");
      return EXIT_FAILURE;
    }

    printf("Send file size : %d\n",fileSize) ;

    char buffer[MAXDATASIZE] ;
    int numbytes = 1;
    while ((numbytes = fread(buffer,sizeof(char),MAXDATASIZE,fptr))) {
        if (send(sockfd,buffer,numbytes,0)==-1) {
            perror("Client: send error");
            return EXIT_FAILURE;
        }
    }

    printf("Send file \n") ;

    /* Réception du code de retour et le traitement du code
        0 en cas de succès sinon 1 et l'erreur parvenue
    */

    char EXIT_Message[MAXDATASIZE+1] ;
    if ( recv(sockfd, EXIT_Message ,MAXDATASIZE, 0) == -1) {
        perror("Client: recv");
        return EXIT_FAILURE;
    }

    char exitStat = EXIT_Message[0] ;
    printf("Receive code %c\n",exitStat );
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
