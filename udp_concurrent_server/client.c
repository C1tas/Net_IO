#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 1234
#define MAXDATASIZE 100

void process(FILE *fp, int sockfd, struct sockaddr_in server);

char *getMessage(char *sendline, int len, FILE *fp);

int main(int argc, char *argv[])
{
  int sockfd,num;
  char buf[MAXDATASIZE];
  struct hostent *he;
  struct sockaddr_in server, peer;
  if(argc != 2){
    printf("Usage: %s <IP Address>\n", argv[0]);
    exit(1);
  }
  if((he = gethostbyname(argv[1])) == NULL){
    perror("gethostbyname() error.\n");
    exit(1);
  }
  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
    perror("socket error.\n");
    exit(1);
  }

  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr = *((struct in_addr*)he->h_addr);
  process(stdin, sockfd, server);
  close(sockfd);
}


void process(FILE *fp, int sockfd, struct sockaddr_in server)
{
  char sendline[MAXDATASIZE], recvline[MAXDATASIZE];
  int num;
  printf("Connected to server.\n");
  printf("Input client's name:");
  if(fgets(sendline, MAXDATASIZE, fp) == NULL){
    printf("\nExit.\n");
    return;
  }

  socklen_t len;
  len = sizeof(server);
  while(getMessage(sendline, MAXDATASIZE, fp) != NULL){
    /* send(sockfd, sendline, strlen(sendline), 0); */
    sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr*)&server, sizeof(server));
    if((num = recvfrom(sockfd, recvline, MAXDATASIZE, 0, (struct sockaddr*)&server, &len)) == 0){
      printf("Server terminated.\n");
      return;
    }

    recvline[num] = '\0';
    printf("Server Messages: %s.\n", recvline);
  }
  printf("\nExit.\n");
}

char* getMessage(char *sendline, int len, FILE *fp)
{
  printf("Input string to server:");
  return(fgets(sendline, MAXDATASIZE, fp));
}
