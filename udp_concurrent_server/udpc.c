#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 1234
#define MAXDATASIZE 1000

char* getMessage(char *sendline, int len, FILE *fp);

int main(int argc, char **argv)
{
  int i;
  int FormatLen;
  int ServerFB;
  int Rst;
  int ServerAddrLen;
  int RecvLen;
  unsigned char RecvBuf[1000];
  unsigned char SendBuf[1000];
  unsigned char tmp[1000];
  int tRst;
  struct sockaddr_in ServerAddr;
  struct sockaddr_in ClinetAddr;
  struct sockaddr_in netaddr;

  if(argc != 2){
    printf("Usage: %s <server_ip>\n", argv[0]);
    return -1;
  }

  ServerFB = socket(AF_INET, SOCK_DGRAM, 0);
  if(ServerFB == -1){
    perror("socket error!\n");
    return -1;
  }

  memset(&ServerAddr, 0, sizeof(struct sockaddr));
  ServerAddr.sin_family = AF_INET;
  ServerAddr.sin_port = htons(SERVER_PORT);

  if(inet_aton(argv[1], &ServerAddr.sin_addr) == 0){
    perror("invalid server_ip.\n");
    return -1;
  }
  memset(ServerAddr.sin_zero, 0, 8);

  Rst = sendto(ServerFB, "C1tas", sizeof("C1tas"),0,
               (struct sockaddr*)&ServerAddr, sizeof(ServerAddr));

  Rst = recvfrom(ServerFB, RecvBuf, 999, 0,
                 (struct sockaddr*)&ServerAddr, &ServerAddrLen);

  printf("RecvBuf = %s\n", RecvBuf);

  char sendline[MAXDATASIZE];
  while(getMessage(sendline, MAXDATASIZE, stdin) != NULL){
    Rst = sendto(ServerFB, sendline, strlen(sendline), 0,
                 (struct sockaddr*)&ServerAddr, sizeof(ServerAddr));
    RecvLen = sizeof(struct sockaddr);

    Rst = recvfrom(ServerFB, RecvBuf, 999, 0,
                   (struct sockaddr*)&ServerAddr, &ServerAddrLen);

    if(Rst == -1){
      perror("recvfrom error!!\n");
      return -1;
    }
    RecvBuf[Rst] = '\0';
    printf("ServerAddr ip %s:%d\n", inet_ntoa(ServerAddr.sin_addr),
           ntohs(ServerAddr.sin_port));
    printf("RecvBuf = %s\n", RecvBuf);

    if (Rst == -1){
      perror("sendto error!!\n");
      return -1;
    }
    if(tRst == -1){
      perror("recvfrom error.\n");
      return -1;
    }
    printf("send ok.\n");
    sleep(1);

  }


  close(ServerFB);
  return 0;

}

char* getMessage(char *sendline, int len, FILE *fp)
{
  printf("Input string to server:\n");
  return(fgets(sendline, MAXDATASIZE, fp));
}
