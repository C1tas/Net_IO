#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#define BUFLEN 255
#define max(x,y) (((x)>(y))?(x):(y))

int main(int argc,char**argv)
{
    struct sockaddr_in peeraddr,localaddr;
    int sockfd,n,maxfd,socklen;
    char msg[BUFLEN+1];
    fd_set infds;
    if(argc!=5){
        printf("%s <dest IP address> <dest port> <source IP address> <source port>\n",argv[0]);
        exit(0);
    }
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd<0){
        fprintf(stderr,"socket creating error in udptalk.c\n");
        exit(1);
    }
    socklen=sizeof(struct sockaddr_in);
    memset(&peeraddr,0,socklen);
    peeraddr.sin_family=AF_INET;
    peeraddr.sin_port=htons(atoi(argv[2]));
    if(inet_pton(AF_INET,argv[1],&peeraddr.sin_addr)<=0){
        printf("wrong dest ip address\n");
        exit(0);
    }
    memset(&localaddr,0,socklen);
    localaddr.sin_family=AF_INET;
    if(inet_pton(AF_INET,argv[3],&localaddr.sin_addr)<=0){
        printf("Wrong source IP address\n");
        exit(0);
    }
    localaddr.sin_port=htons(atoi(argv[4]));
    if(bind(sockfd,(struct sockaddr *)&localaddr,socklen)<0){
        fprintf(stderr,"bind local address error in udptalk.c\n");
        exit(2);
    }
    connect(sockfd,(struct sockaddr*)&peeraddr,socklen);
    for(;;)
    {
        FD_ZERO(&infds);
        FD_SET(fileno(stdin),&infds);
        FD_SET(sockfd,&infds);
        maxfd=max(fileno(stdin),sockfd)+1;
        if(select(maxfd,&infds,NULL,NULL,NULL)==-1)
        {
            fprintf(stderr,"select error in udptalk.c\n");
            exit(3);
        }
        if(FD_ISSET(sockfd,&infds))
        {
            n=read(sockfd,msg,BUFLEN);
            if((n==-1)||(n==0)){
                printf("peer closed\n");
                exit(0);
            }
            else   {
                msg[n]=0;
                printf("peer:%s",msg);
            }
        }
        if(FD_ISSET(fileno(stdin),&infds))
        {
            if(fgets(msg,BUFLEN,stdin)==NULL){
                printf("talk over!\n");
                exit(0);
            }
            write(sockfd,msg,strlen(msg));
            printf("sent:%s",msg);
        }
    }
}
