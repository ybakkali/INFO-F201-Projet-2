//Server

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
//#include <sys/wait.h>
//#include <signal.h>
#include <sys/stat.h>

#define MYPORT 5555
#define BACKLOG 20
#define MAXDATASIZE 1024

void createDir(char **dir,int index) {
  if (index < 4 ) {
      struct stat st = {0};
      int x = 0 ;
      if (stat(dir[index], &st) == -1) {
          x = mkdir(dir[index], 0750);
      }
      if (x == -1) {
        perror("Error : mkdir");
      }
      createDir(dir,index+1) ;
  }
}


int main(int argc, char *argv[])
{
  if (argc != 2) {
      fprintf(stderr,"usage: synopsis erroné\n");
      return EXIT_FAILURE;
  }

  int server_sockfd, client_sockfd, numbytes;
  // Ecouter sur server_sockfd
  // Se connecter sur client_sockfd
  struct sockaddr_in server_addr;
  // Information de l'adresse du serveur
  struct sockaddr_in client_addr;
  // Information de l'adresse du client
  char buf[MAXDATASIZE];
  //Stocker

  server_sockfd = socket(PF_INET,SOCK_STREAM, 0) ;

  if (server_sockfd == -1) {
    perror("Serveur: socket");
    return EXIT_FAILURE;
  }

  int yes=1;
  if (setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
  {
    perror("Serveur: setsockopt");
    return EXIT_FAILURE;
  }


  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(MYPORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  memset(&(server_addr.sin_zero), '\0', 8);


  if (bind(server_sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
  {
    perror("Serveur: bind");
    return EXIT_FAILURE;
  }

  if (listen(server_sockfd, BACKLOG) == -1) {
    perror("Serveur: listen");
    return EXIT_FAILURE;
  }

  unsigned int sin_size = sizeof(struct sockaddr_in);

  while(1){

    client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_addr,&sin_size);

    if (client_sockfd == -1) {
      perror("Serveur: accept");
    }

    printf("Serveur:  connection recue du client %s\n",inet_ntoa(client_addr.sin_addr));

    if (fork()==0) {
      /* this is the child process */
      close(server_sockfd);

      int sizes[] = {0,4,2,2,0} ;
      int i ;
      for (i = 0 ; i < 2 ; ++i) {
          unsigned long size ;
          int n = recv(client_sockfd, &size, sizeof(long), 0);
          if ( n == -1) {
              perror("Server: recv");
              return EXIT_FAILURE;
          }

          printf("Recieve size %lu \n",size) ;
          if (i==0) {
            sizes[0] = size;
          }
          else {
            sizes[4] = size ;
          }
      }
      char *path = argv[1] ;
      char *dirUser = malloc(100 * sizeof(char)) ;
      char *dirUserYear = malloc(100 * sizeof(char)) ;
      char *dirUserYearMonth = malloc(100 * sizeof(char)) ;
      char *dirUserYearMonthDay = malloc(100 * sizeof(char)) ;
      char filename[sizes[4]] ;
      int counter ;
      for (counter = 0 ; counter < 5 ; ++counter ) {
          printf("message size %d \n",sizes[counter]) ;
          numbytes = recv(client_sockfd, buf, sizes[counter], 0) ;
          if ( numbytes == -1) {
              perror("Server: recv");
              return EXIT_FAILURE;
          }

          buf[numbytes] = '\0';
          if (counter < 4 ) {
            strcat(path,"/") ;
            strcat(path,buf) ;
          }
          switch (counter) {
            case 0 :
              strcpy(dirUser,path) ;
              break ;
            case 1 :
              strcpy(dirUserYear,path) ;
              break ;
            case 2 :
              strcpy(dirUserYearMonth,path) ;
              break ;
            case 3 :
              strcpy(dirUserYearMonthDay,path) ;
              break ;
            case 4 :
              strcpy(filename,buf) ;
              break ;
            default :
              printf("OK\n");
          }
          printf("Message recu du client: %s \n",buf);
      }
      printf ("%s\n%s\n%s\n%s\n",dirUser,dirUserYear,dirUserYearMonth,dirUserYearMonthDay) ;
      char *dir[4] = {dirUser,dirUserYear,dirUserYearMonth,dirUserYearMonthDay} ;
      createDir(dir,0) ;
      printf("%s\n",filename) ;

      char message[] = {"Bien reçu\n"} ;
      if (send(client_sockfd, message ,strlen(message),0)==-1) {
      	perror("Serveur: send");
      	return EXIT_FAILURE;
      }

      return EXIT_SUCCESS;
    }

    close(client_sockfd);
  }

  return EXIT_SUCCESS;
}
