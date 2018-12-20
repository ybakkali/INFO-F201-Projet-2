// BAKKALI Yahya
// 00445166
// Server code

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
#include <sys/stat.h>

#define PORT 5555
#define BACKLOG 20
#define MAXDATASIZE 1024

int makeDirectory(char *dir ,int sockfd) {

    /* Fonction pour créer un repertoire */

    int dirStat ;
    struct stat st = {0};

    if (stat(dir, &st) == -1) {
    // Le repertoire n'existe pas
        dirStat = mkdir(dir, 0750);
        // Créer le repertoire
        if (dirStat == -1) {
            // Le repertoire n'est pas créer
            perror("Error : mkdir");
            char errorMessage[MAXDATASIZE] ;
            strcpy(errorMessage,"1Error : mkdir") ;
            if (send(sockfd,errorMessage,MAXDATASIZE,0)==-1) {
                // Envoyer au client l'erreur parvenue
                perror("Server: send");
                return EXIT_FAILURE;
            }
        }
    }
    return EXIT_SUCCESS ;
}

void makePATH(char **dir,char *PoolV2PATH,char *finalPATH,int sockfd) {

    /* Fonction pour créer l'arborescence dans le pool v2 */

    char dirPATH[MAXDATASIZE] ;
    strcpy(dirPATH,PoolV2PATH) ;
    int index  ;
    for ( index = 0 ; index < 4 ; index++ ) {
        strcat(dirPATH,"/") ;
        strcat(dirPATH,dir[index]) ;
        makeDirectory(dirPATH,sockfd) ;
    }
    strcat(dirPATH,"/") ;
    strcat(dirPATH,dir[4]) ;
    strcpy(finalPATH,dirPATH) ;
}

int makeCopy(char *path , FILE* ftmp , int sockfd) {

    /* Fonction pour copier le fichier qui se trouve dans
     "/tmp/file" vers le bon endroit dans pool v2
     "/chemin/vers/poolv2/username/year/month/day/file"
    */

    rewind(ftmp);
    // Début de ficher
    FILE *fptr ;
    printf("Moving file to : %s\n",path) ;
    fptr = fopen(path , "w");
    if ( fptr == NULL ) {
        perror("Error : File not create");
        if (send(sockfd, "1Error : File not create" ,MAXDATASIZE,0)==-1) {
            perror("Server: send");
            return EXIT_FAILURE;
        }
        return EXIT_FAILURE;
    }

    int numbytes = 1;
    char buff[MAXDATASIZE];
    while ((numbytes = fread(buff,sizeof(char),MAXDATASIZE,ftmp))) {
        fwrite (buff,sizeof(char),numbytes,fptr);
    }
    fclose(fptr) ;
    return EXIT_SUCCESS ;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"usage: synopsis erroné\n");
        return EXIT_FAILURE;
    }

    int server_sockfd, client_sockfd ;
    // Ecouter sur server_sockfd et se connecter sur client_sockfd
    struct sockaddr_in server_addr;
    // Information de l'adresse du serveur
    struct sockaddr_in client_addr;
    // Information de l'adresse du client

    if ((server_sockfd = socket(PF_INET,SOCK_STREAM, 0))== -1) {
        perror("Server: socket");
        return EXIT_FAILURE;
    }

    int yes=1;
    if (setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        perror("Server: setsockopt");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), '\0', 8);


    if (bind(server_sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
        perror("Server: bind");
        return EXIT_FAILURE;
    }

    if (listen(server_sockfd, BACKLOG) == -1) {
        perror("Server: listen");
        return EXIT_FAILURE;
    }

    unsigned int sin_size = sizeof(struct sockaddr_in);

    while(1){
        /* le processus du serveur */
        if ((client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_addr,&sin_size)) == -1) {
            perror("Server: accept");
        }

        printf("Server: connection received from the client %s\n",inet_ntoa(client_addr.sin_addr));

        if (fork()==0) {
            /* le processus de chaque client */
            close(server_sockfd);
            // Fermer le socket du serveur
            char errorMessage[MAXDATASIZE] ;
            // Liste pour stocker le message d'erreur
            int usernameSize, filenameSize, numbytes ;
            // Variables pour stocker les tailles

            /* Réception de la taille d'utilisateur et du nom de fichier */

            if (recv(client_sockfd,&usernameSize,sizeof(int),0) == -1){
                perror("Server: recv");
                return EXIT_FAILURE;
            }

            printf("Receive username size : %d \n",usernameSize) ;

            if (recv(client_sockfd,&filenameSize,sizeof(int),0) == -1) {
                perror("Server: recv");
                return EXIT_FAILURE;
            }

            printf("Receive filename size : %d \n",filenameSize) ;

            /* Des listes pour stocker les informations :
                - nom d'utilisateur : taille reçue
                - année : 4
                - mois : 2
                - jour : 2
                - nom du ficher : taille reçue
             */

            char username[usernameSize+1] ;
            char year[5] ;
            char month[3] ;
            char day[3] ;
            char filename[filenameSize+1] ;

            /* Réception du nom d'utilisateur,année,mois,jour,nom du ficher */

            if ((numbytes=recv(client_sockfd,username,usernameSize,0)) == -1) {
                strcpy(errorMessage,"1Error : Username not received\n") ;
                if (send(client_sockfd,errorMessage,MAXDATASIZE,0)==-1) {
                    // Envoyer au client l'erreur parvenue
                    perror("Server: send");
                    return EXIT_FAILURE;
                }
                perror("Server: recv");
                return EXIT_FAILURE;
            }
            username[numbytes] = '\0';
            printf("Receive username : %s \n",username);

            if ( (numbytes=recv(client_sockfd, year, 4 , 0)) == -1) {
                strcpy(errorMessage,"1Error : Year not received\n") ;
                if (send(client_sockfd,errorMessage,MAXDATASIZE,0)==-1) {
                    // Envoyer au client l'erreur parvenue
                    perror("Server: send");
                    return EXIT_FAILURE;
                }
                perror("Server: recv");
                return EXIT_FAILURE;
            }
            year[numbytes] = '\0';
            printf("Receive year : %s \n",year);

            if ( (numbytes=recv(client_sockfd, month, 2 , 0)) == -1) {
                strcpy(errorMessage,"1Error : Month not received\n") ;
                if (send(client_sockfd,errorMessage,MAXDATASIZE,0)==-1) {
                    // Envoyer au client l'erreur parvenue
                    perror("Server: send");
                    return EXIT_FAILURE;
                }
                perror("Server: recv");
                return EXIT_FAILURE;
            }
            month[numbytes] = '\0';
            printf("Receive month : %s \n",month);

            if ( (numbytes=recv(client_sockfd, day, 2 , 0)) == -1) {
                strcpy(errorMessage,"1Error : Day not received\n") ;
                if (send(client_sockfd,errorMessage,MAXDATASIZE,0)==-1) {
                    // Envoyer au client l'erreur parvenue
                    perror("Server: send");
                    return EXIT_FAILURE;
                }
                perror("Server: recv");
                return EXIT_FAILURE;
            }
            day[numbytes] = '\0';
            printf("Receive day : %s \n",day);

            if ( (numbytes=recv(client_sockfd, filename, filenameSize , 0)) == -1) {
                strcpy(errorMessage,"1Error : Filename not received\n") ;
                if (send(client_sockfd,errorMessage,MAXDATASIZE,0)==-1) {
                    // Envoyer au client l'erreur parvenue
                    perror("Server: send");
                    return EXIT_FAILURE;
                }
                perror("Server: recv");
                return EXIT_FAILURE;
            }
            filename[numbytes] = '\0';
            printf("Receive filename : %s \n",filename);
            ////////

            char tmpPATH[MAXDATASIZE+1] = "/tmp/" ;
            // Path où le fichier temporaire va être stocker
            strcat(tmpPATH,filename) ;
            FILE *ftmp ;
            // Pointeur vers le fichier temporaire
            ftmp = fopen(tmpPATH , "w+");
            // Créer le fichier temporaire
            if (ftmp == NULL) {
                perror("Error : No such file");
                return EXIT_FAILURE;
            }
            ///////
            int recvFileSize ;
            // Taille du fichier photo envoyé

            if ((numbytes=recv(client_sockfd,&recvFileSize,sizeof(int),0))== -1) {
                perror("Server: recv");
                return EXIT_FAILURE;
            }
            // Réception de la taille du fichier
            printf("Receive file size : %d \n",recvFileSize);
            ///////
            char buffer[MAXDATASIZE+1];
            // Liste pour stocker les informations
            int fileSize = 0 ;
            // La taille finale du fichier photo reçue
            while (fileSize < recvFileSize ) {
                if ((numbytes=recv(client_sockfd,buffer,MAXDATASIZE,0))==-1) {
                    // Reception interrompue
                    perror("Server: recv");
                    return EXIT_FAILURE;
                }
                fwrite (buffer,sizeof(char),numbytes,ftmp);
                fileSize += numbytes ;
            }

            if (fileSize < recvFileSize) {
                fprintf(stderr,"Damaged file") ;
                strcpy(errorMessage,"1Error : Damaged file") ;
                if (send(client_sockfd,errorMessage,MAXDATASIZE,0)==-1) {
                    // Envoyer au client l'erreur parvenue
                    perror("Server: send");
                    return EXIT_FAILURE;
                }
                return EXIT_FAILURE;
            }

            printf("Receive file\n") ;
            ///////
            char finalPATH[MAXDATASIZE] ;
            char* dir[5] = {username,year,month,day,filename} ;
            makePATH(dir,argv[1],finalPATH,client_sockfd);
            // Création de l’arborescence
            makeCopy(finalPATH,ftmp,client_sockfd) ;
            // Enregistrement de la photographie
            fclose(ftmp) ;
            // Fermer le fichier temporaire
            remove(tmpPATH);
            // Supprimer le fichier temporaire

            /* Le processus est bien exécuté et retourne le code 0 */

            char message[MAXDATASIZE] = {"0"} ;
            if (send(client_sockfd,message,MAXDATASIZE,0)==-1) {
                perror("Server: send");
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
        close(client_sockfd);
        // Fermer le socket du client
    }
    return EXIT_SUCCESS;
}
