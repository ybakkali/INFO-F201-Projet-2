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

#define MYPORT 5555
#define BACKLOG 20
#define MAXDATASIZE 1024

int main()
{
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
    //test

    numbytes = recv(client_sockfd, buf, MAXDATASIZE-1, 0) ;
    if ( numbytes == -1) {
        perror("Server: recv");
        return EXIT_FAILURE;
    }

    buf[numbytes] = '\0';

    printf("Message recu du client: %s \n",buf);

    //test
    if (fork()==0) {
      /* this is the child process */
      close(server_sockfd);

      if (send(client_sockfd, "Bonjour client!\n",16,0)==-1) {
      	perror("Serveur: send");
      	return EXIT_FAILURE;
      }

      return EXIT_SUCCESS;
    }

    close(client_sockfd);
  }

  return EXIT_SUCCESS;
}
