#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 1234
#define BACKLOG 5
#define MAXDATASIZE 100

void process_cli(int connfd, struct sockaddr_in client);

int main()
{
  int sockfd;
  pid_t pid;
  struct sockaddr_in server;
  struct sockaddr_in client;
  int len;
  char buf[MAXDATASIZE];

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
    perror("Create socket error.\n");
    exit(1);
  }

  /* int opt = SO_REUSEADDR; */
  /* setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); */
  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if(bind(sockfd, (struct sockaddr*)&server, sizeof(server)) == -1){
    perror("Bind() error.\n");
    exit(1);
  }

  len = sizeof(client);

  while(1){
    int num;
    num = recvfrom(sockfd, buf, MAXDATASIZE, 0, (struct sockaddr*)&client, &len);
    printf("Server inital success~");
    if(num < 0){
      perror("recvfrom error.\n");
      exit(1);
    }

    if((pid = fork()) > 0){
      close(sockfd);
      continue;
    }
    else if(pid == 0){
      close(sockfd);
      process_cli(sockfd, client);
      exit(0);
    }
    else{
      perror("fock() error.\n");
      exit(0);
    }
  }
  close(sockfd);

}

void process_cli(int sockfd, struct sockaddr_in client)
{
  int num;
  socklen_t len;
  char recvbuf[MAXDATASIZE], sendbuf[MAXDATASIZE], cli_name[MAXDATASIZE];
  printf("You got a connection from %s, it's port is %d.", inet_ntoa(client.sin_addr), htons(client.sin_port));
  num = recv(sockfd, cli_name, MAXDATASIZE, 0);

  if(num == 0){
    close(sockfd);
    printf("Client disconnected.\n");
    return;
  }
  cli_name[num -1] = '\0';
  printf("Client name is %s.\n", cli_name);

  len = sizeof(client);
  while(num = recvfrom(sockfd, recvbuf, MAXDATASIZE, 0, (struct sockaddr*)&client, &len)){
    recvbuf[num] = '\0';
    printf("Received client(%s) message: %s", cli_name, recvbuf);

    int  i = 0;
    char tmpbuf[MAXDATASIZE];
    for (i = 0; i < num - 1; i++){
      tmpbuf[num - 2 - i] = recvbuf[i];
    }

    tmpbuf[num -1] = '\0';
    sendto(sockfd, tmpbuf, strlen(tmpbuf), 0, (struct sockaddr*)&client, len);
    if(strncmp(recvbuf, "q", 1) == 0)
      break;
  }
  close(sockfd);
}
