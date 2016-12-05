#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define SERVER_PORT 1234

int main(int argc, char *argv[])
{
  int Rst;
  int RecvLen;
  int RecvRandomLen;
  unsigned char RecvBuf[1000];
  unsigned char RecvRandomBuf[1000];

  struct sockaddr_in ServerAddr;
  struct sockaddr_in ClientAddr;

  int ServerFB;
  int ServerRandomFB;

  ServerFB = socket(AF_INET, SOCK_DGRAM, 0);
  if (ServerFB == -1){
    perror("socket error.\n");
    return -1;
  }
  memset(&ServerAddr, 0, sizeof(struct sockaddr));
  ServerAddr.sin_family = AF_INET;
  ServerAddr.sin_port = htons(SERVER_PORT);
  ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(ServerAddr.sin_zero, 0, 8);

  Rst = bind(ServerFB, (const struct sockaddr*)&ServerAddr, sizeof(struct sockaddr));
  if(Rst == -1){
    perror("bind error.\n");
    return -1;
  }

  socklen_t ClientAddrLen;
  while(1){
    ClientAddrLen = sizeof(struct sockaddr);
    RecvLen = recvfrom(ServerFB, RecvBuf,
                       999, 0, (struct sockaddr*)&ClientAddr, &ClientAddrLen);
    if(RecvLen < 0){
      perror("recv from client error.\n");
      return -1;
    }
    printf("RecvUdpAddr ip:%s:%d\n", inet_ntoa(ClientAddr.sin_addr),
           ntohs(ClientAddr.sin_port));
    RecvBuf[RecvLen] = '\0';
    printf("Client Name is : %s\n", RecvBuf);
    if (fork() == 0){
      break;
    }
    printf("Sub progress~~~~~~\n");
  }

  printf("Enter Fork.\n");
  ServerRandomFB = socket(AF_INET, SOCK_DGRAM, 0);
  if(ServerRandomFB == -1){
    perror("Random socket error.\n");
    return -1;
  }

  printf("RecvUdpAddr ip:%s:%d\n", inet_ntoa(ClientAddr.sin_addr),
         ntohs(ClientAddr.sin_port));
  Rst = sendto(ServerRandomFB, "welcome udp server", strlen("welcome udp server"), 0,
               (struct sockaddr*)&ClientAddr, sizeof(struct sockaddr));

  if(Rst == -1){
    perror("Random Server send error.\n");
    return -1;
  }
  else{
    //
  }

  while(1){
    RecvRandomLen = recvfrom(ServerRandomFB, RecvRandomBuf, 999, 0,
                             (struct sockaddr*)&ClientAddr,
                             &ClientAddrLen);
    unsigned char tmp[1000];
    int i;
    for(i = 0; i < RecvRandomLen; i++){
      tmp[RecvRandomLen - 1 - i] = RecvRandomBuf[i];
    }
    tmp[RecvRandomLen] = '\0';
    printf("tmp = %s\n", tmp);
    sendto(ServerRandomFB, tmp, 999, 0,
           (struct sockaddr*)&ClientAddr, sizeof(struct sockaddr));
    if(RecvRandomLen < 0){
      perror("Random receive error.\n");
      return -1;
    }
    RecvRandomBuf[RecvRandomLen] = '\0';
    printf("ip: %s:%d random receive buf = %s\n\n",
           inet_ntoa(ClientAddr.sin_addr),
           ntohs(ClientAddr.sin_port),
           RecvRandomBuf);
  }
  close(ServerFB);
  return 0;
}

