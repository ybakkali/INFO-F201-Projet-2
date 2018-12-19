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

  int  dirStat ;
  printf("%s \n",dir) ;
  struct stat st = {0};

  if (stat(dir, &st) == -1) {
    // Le repertoire n'existe pas
    dirStat = mkdir(dir, 0750);
    // Créer le repertoire
    if (dirStat == -1) {
      // Le repertoire n'est pas créer
      perror("Error : mkdir");
      if (send(sockfd, "1Error : mkdir" ,MAXDATASIZE,0)==-1) {
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
  FILE *fptr ;
  printf("Move file to : %s\n",path) ;
  fptr = fopen(path , "w");
  if ( fptr == NULL )
  {
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
    //printf("%d\n",numbytes) ;
    fwrite (buff,sizeof(char),numbytes,fptr);
  }
  fclose(fptr) ;
  return EXIT_SUCCESS ;
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
      fprintf(stderr,"Error : synopsis\n");
      return EXIT_FAILURE;
  }

  int server_sockfd, client_sockfd ;
  // Ecouter sur server_sockfd et se connecter sur client_sockfd
  struct sockaddr_in server_addr;
  // Information de l'adresse du serveur
  struct sockaddr_in client_addr;
  // Information de l'adresse du client
  char buffer[MAXDATASIZE];
  // Liste pour stocker les informations

  if ((server_sockfd = socket(PF_INET,SOCK_STREAM, 0)) == -1) {
    perror("Server: socket");
    return EXIT_FAILURE;
  }

  int yes=1;
  if (setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
  {
    perror("Server: setsockopt");
    return EXIT_FAILURE;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  memset(&(server_addr.sin_zero), '\0', 8);

  if (bind(server_sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
  {
    perror("Server: bind");
    return EXIT_FAILURE;
  }

  if (listen(server_sockfd, BACKLOG) == -1) {
    perror("Server: listen");
    return EXIT_FAILURE;
  }

  unsigned int sin_size = sizeof(struct sockaddr_in);
  while(1){
    /* Processus du  pere */
    client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_addr,&sin_size);
    if (client_sockfd == -1) {
      perror("Server: accept");
    }

    printf("Server: connection received from the client %s\n",inet_ntoa(client_addr.sin_addr));

    if (fork()==0) {
      /* Processus du fils */
      close(server_sockfd);
      // Fermer le socket du serveur
      char username [MAXDATASIZE] ;
      // Liste pour stocker le "username"
      char year [MAXDATASIZE] ;
      // Liste pour stocker le "year"
      char month [MAXDATASIZE] ;
      // Liste pour stocker le "month"
      char day [MAXDATASIZE] ;
      // Liste pour stocker le "day"
      char filename[MAXDATASIZE] ;
      // Liste pour stocker le "filename"
      int counter , numbytes ;
      for (counter = 0 ; counter < 5 ; ++counter ) {
          numbytes = recv(client_sockfd, buffer, MAXDATASIZE, 0) ;
          if ( numbytes == -1) {
            if (send(client_sockfd, "1Error : Username not received\nDate not received\nFilename not received\n" ,MAXDATASIZE,0)==-1) {
              // Envoyer au client l'erreur parvenue
              perror("Server: send");
              return EXIT_FAILURE;
            }
            perror("Server: recv");
            return EXIT_FAILURE;
          }

          buffer[numbytes] = '\0';

          switch (counter) {
            case 0 : strcpy(username,buffer) ; break ;
            case 1 : strcpy(year,buffer) ; break ;
            case 2 : strcpy(month,buffer) ; break ;
            case 3 : strcpy(day,buffer) ; break ;
            case 4 : strcpy(filename,buffer) ; break ;
          }
      }


      if (send(client_sockfd,"Username received\nDate received\nFilename received\n" ,MAXDATASIZE,0)==-1) {
      	perror("Server: send");
      	return EXIT_FAILURE;
      }


      char tmpPath[MAXDATASIZE] = "/tmp/" ;
      // Path où le fichier temporaire va être stocker
      strcat(tmpPath,filename) ;
      printf("%s\n",tmpPath) ;
      FILE *ftmp ;
      // Pointeur vers le fichier temporaire
      ftmp = fopen(tmpPath , "w+");
      // Créer le fichier temporaire
      if ( ftmp == NULL )
      {
         perror("Error : No such file");
         return EXIT_FAILURE;
      }
      unsigned long recvFileSize ;
      // Taille du fichier photo qu'on va recevoir
      int numbytes_ ;
      // Numero des bytes reçues
      numbytes_ = recv(client_sockfd, &recvFileSize, sizeof(long), 0) ;
      if ( numbytes_ == -1) {
          perror("Server: recv");
          return EXIT_FAILURE;
      }

      printf("File size : %lu \n",recvFileSize) ;
      unsigned long fileSize = 0 ;
      // La taille finale du fichier photo reçue
      int numbytes__ ;
      // Numero des bytes reçues
      while (fileSize < recvFileSize ) {
        numbytes__ = recv(client_sockfd, buffer, MAXDATASIZE , 0) ;
        if ( numbytes__ == -1) {
            // Reception interrompue
            perror("Server: recv");
            return EXIT_FAILURE;
        }
        fwrite (buffer,sizeof(char),numbytes__,ftmp);
        fileSize += numbytes__ ;
      }

      if (fileSize < recvFileSize) {
        fprintf(stderr,"Damaged file") ;
        if (send(client_sockfd, "1Error : Damaged file" ,MAXDATASIZE,0)==-1) {
          // Envoyer au client l'erreur parvenue
          perror("Server: send");
          return EXIT_FAILURE;
        }
        return EXIT_FAILURE;
      }

      char *dir[5] = {username,year,month,day,filename} ;
      char finalPATH[MAXDATASIZE] ;
      makePATH(dir,argv[1],finalPATH,client_sockfd);
      makeCopy(finalPATH,ftmp,client_sockfd) ;

      if (send(client_sockfd, "0\0" ,MAXDATASIZE,0)==-1) {
        // Envoi du code 0 en cas de succès
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
