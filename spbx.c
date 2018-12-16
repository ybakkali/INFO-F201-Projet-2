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

void makeDirectories(char **dir,int index) {
  if (index < 4 ) {
      printf("%s \n",dir[index]) ;
      struct stat st = {0};
      int x = 0 ;
      if (stat(dir[index], &st) == -1) {
          x = mkdir(dir[index], 0750);
      }
      if (x == -1) {
        perror("Error : mkdir");
      }
      makeDirectories(dir,index+1) ;
  }
}


int main(int argc, char *argv[])
{
  if (argc != 2) {
      fprintf(stderr,"Error : synopsis\n");
      return EXIT_FAILURE;
  }

  int server_sockfd, client_sockfd ;
  // Ecouter sur server_sockfd
  // Se connecter sur client_sockfd
  struct sockaddr_in server_addr;
  // Information de l'adresse du serveur
  struct sockaddr_in client_addr;
  // Information de l'adresse du client
  char buffer[MAXDATASIZE];
  //Stocker

  server_sockfd = socket(PF_INET,SOCK_STREAM, 0) ;
  if (server_sockfd == -1) {
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
  server_addr.sin_port = htons(MYPORT);
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

    client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_addr,&sin_size);
    if (client_sockfd == -1) {
      perror("Server: accept");
    }

    printf("Server: connection received from the client %s\n",inet_ntoa(client_addr.sin_addr));

    if (fork()==0) {
      /* this is the child process */
      close(server_sockfd);

      int messageSize[] = {0,4,2,2,0,0} ;
      unsigned long size ;
      int i , n  ;
      for (i = 0 ; i < 3 ; ++i) {
          n = recv(client_sockfd, &size, sizeof(long), 0);
          if ( n == -1) {
              perror("Server: recv");
              return EXIT_FAILURE;
          }

          printf("Recieve size %lu \n",size) ;
          if (i==0) {
            messageSize[0] = size;
          }
          else if (i==1) {
            messageSize[4] = size ;
          }
          else {
            messageSize[5] = size ;
          }
      }
      char *path = argv[1] ;
      int len = strlen(argv[1]) + messageSize[0] + 1 ;
      char dirUser[len] ;
      len += 5 ;
      char dirUserYear[len] ;
      len += 3 ;
      char dirUserYearMonth[len] ;
      len += 3 ;
      char dirUserYearMonthDay[len] ;
      char filename[messageSize[4]] ;
      int counter , numbytes ;
      for (counter = 0 ; counter < 5 ; ++counter ) {
          printf("Message size %d \n",messageSize[counter]) ;
          numbytes = recv(client_sockfd, buffer, messageSize[counter], 0) ;
          if ( numbytes == -1) {
              perror("Server: recv");
              return EXIT_FAILURE;
          }

          buffer[numbytes] = '\0';
          printf("Message received from the customer : %s \n",buffer);
          strcat(path,"/") ;
          strcat(path,buffer) ;

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
              strcpy(filename,buffer) ;
              break ;
            default :
              printf("OK\n");
          }
      }
      char *dir[4] = {dirUser,dirUserYear,dirUserYearMonth,dirUserYearMonthDay} ;
      makeDirectories(dir,0) ;

      char message[] = {"Well received\n"} ;
      if (send(client_sockfd, message ,strlen(message),0)==-1) {
      	perror("Server: send");
      	return EXIT_FAILURE;
      }

      FILE *fptr ;
      printf("%s\n",path) ;
      fptr = fopen(path , "w");
      if ( fptr == NULL )
      {
         perror("Error : No such file");
         return EXIT_FAILURE;
      }

      int fileSize = 0 ;
      int nb ;
      while (fileSize < messageSize[5] ) {
        nb = recv(client_sockfd, buffer, MAXDATASIZE , 0) ;
        if ( nb == -1) {
            perror("Server: recv");
            return EXIT_FAILURE;
        }
        fwrite (buffer,sizeof(char),nb,fptr);
        fileSize += nb ;
      }

      fclose(fptr) ;
      return EXIT_SUCCESS;
    }

    close(client_sockfd);
  }

  return EXIT_SUCCESS;
}
