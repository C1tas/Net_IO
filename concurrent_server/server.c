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
  int listenfd, connfd;
  pid_t pid;
  struct sockaddr_in server;
  struct sockaddr_in client;
  int len;

  if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("Create socket error.\n");
    exit(1);
  }

  int opt = SO_REUSEADDR;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(listenfd, (struct sockaddr*)&server, sizeof(server)) == -1){
    perror("Bind() error.\n");
    exit(1);
  }

  if(listen(listenfd, BACKLOG) == -1){
    perror("listen() error.\n");
    exit(1);
  }

  len = sizeof(client);

  while(1){
    if((connfd = accept(listenfd, (struct sockaddr*)&client, &len)) == -1){
      perror("accept() error.\n");
      exit(1);
    }
    if((pid = fork()) > 0){
      close(connfd);
      continue;
    }
    else if(pid == 0){
      close(listenfd);
      process_cli(connfd, client);
      exit(0);
    }
    else{
      perror("fock() error.\n");
      exit(0);
    }
  }
  close(listenfd);

}

void process_cli(int connfd, struct sockaddr_in client)
{
  int num;
  char recvbuf[MAXDATASIZE], sendbuf[MAXDATASIZE], cli_name[MAXDATASIZE];
  printf("You got a connection from %s, it's port is %d.", inet_ntoa(client.sin_addr), htons(client.sin_port));
  num = recv(connfd, cli_name, MAXDATASIZE, 0);

  if(num == 0){
    close(connfd);
    printf("Client disconnected.\n");
    return;
  }
  cli_name[num -1] = '\0';
  printf("Client name is %s.\n", cli_name);

  while(num = recv(connfd, recvbuf, MAXDATASIZE, 0)){
    recvbuf[num] = '\0';
    if(strncmp(recvbuf, "quit", 4) == 0)
    {
      break;
    }
    printf("Received client(%s) message: %s", cli_name, recvbuf);

    int  i = 0;
    char tmpbuf[MAXDATASIZE];
    for (i = 0; i < num - 1; i++){
      tmpbuf[num - 2 - i] = recvbuf[i];
    }

    /* int i = 0; */
    /* for (i = 0; i < num-1; i++){ */
    /*   if(recvbuf[i] == ' '){ */
    /*     sendbuf[i] = recvbuf[i]; */
    /*     continue;  */
    /*   } */
    /*   if((recvbuf[i] >= 'a' && recvbuf[i] <= 'z') || (recvbuf[i] >= 'A' && recvbuf[i] <= 'Z')){ */
    /*     recvbuf[i] = recvbuf[i] + 3; */
    /*     if((recvbuf[i] > 'Z' && recvbuf[i] <= 'Z'+3) || (recvbuf[i] > 'z')){ */
    /*       recvbuf[i] = recvbuf[i] - 26; */
    /*     } */
    /*     sendbuf[i] = recvbuf[i]; */
    /*   } */
    /* } */

    tmpbuf[num -1] = '\0';
    send(connfd, tmpbuf, strlen(tmpbuf), 0);
  }
  close(connfd);
}
