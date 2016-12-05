#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 1234
#define MAXDATASIZE 100
char* getMessage(char *sendline, int len, FILE *fp);


int main(int argc, char *argv[])
{
    int sockfd, num;
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in server, peer;
    if(argc != 2)
    {
        printf("Usage: %s <IP Address>\n", argv[0]);
        exit(1);
    }

    if((he = gethostbyname(argv[1])) == NULL)
    {
        printf("gethostbyname() error.\n");
        exit(1);
    }

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket() error.\n");
        exit(1);
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr*)he->h_addr);
    socklen_t len;
    len = sizeof(server);
    char sendline[MAXDATASIZE], recvline[MAXDATASIZE];
    while(strncmp(getMessage(sendline, MAXDATASIZE, stdin), "quit", 4) != 0)
    {
        sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr*)&server, sizeof(server));
        if((num = recvfrom(sockfd, recvline, MAXDATASIZE, 0, (struct sockaddr*)&peer, &len)) == -1)
        {
            printf("recvfrom() error.\n)");
            exit(1);
        }

        if(len != sizeof(server) || memcmp((const void *)&server, (const void*)&peer, len) != 0)
        {
            printf("Recevice message from other server.\n");
            continue;
        }

        recvline[num] = '\0';
        printf("Server Message:%s.\n", recvline);


    }
    sendto(sockfd, "quit", strlen("quit"), 0, (struct sockaddr*)&server, sizeof(server));
    close(sockfd);
}

char* getMessage(char *sendline, int len, FILE *fp)
{
  printf("Input string to server:");
  return(fgets(sendline, MAXDATASIZE, fp));
}
