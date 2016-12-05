#include <netinet/in.h>

#include <sys/socket.h>

#include <stdlib.h>

#include <string.h>

#include <sys/types.h>

#include <stdio.h>

#include <arpa/inet.h>

#include <unistd.h>

#include <pthread.h>



/*宏定义*/

#define BUFFER_SIZE 1024

#define PORT 4001



/*函数声明*/

void *function(void *arg);

char* getMessage(char *sendline, int len, FILE *fp)
{
    printf("Input string to server:");
    return(fgets(sendline, BUFFER_SIZE, fp));
}


/*结构体定义*/

struct ARG

{

    int sockfd;
  
    int len;  
  
};  
  
   
  
struct sockaddr_in client;      //定义全局结构体变量



/*主函数*/

int main(void)

{

    /*定义变量*/
  
    pthread_t tid;

    struct ARG *arg;

    int sockfd,len;

    char se_mes[BUFFER_SIZE];

    struct sockaddr_in server;



    /*调用套接字函数*/

    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    if(sockfd==-1)

    {

        perror("socket() error");

        exit(0);

    }

    //printf("Socket success!/n");



    /*初始化server套接字地址结构，并对地址结构中的成员变量赋值*/

    bzero(&server,sizeof(server));

    server.sin_family=AF_INET;

    server.sin_port=htons(PORT);

    server.sin_addr.s_addr=htonl(INADDR_ANY);



    if(bind(sockfd,(struct sockaddr *)&server,sizeof(server))==-1)

    {

        perror("bind() error");

        exit(0);

    }

    //printf("Bind success!/n");

    printf("等待客户端连接......\n");



    len=sizeof(client);

    arg=(struct ARG *)malloc(sizeof(struct ARG));     //给结构体分配内存空间



    /*给结构体成员变量赋值*/

    arg->sockfd=sockfd;

    arg->len=len;



    /*产生子线程*/

    if(pthread_create(&tid,NULL,function,(void *)arg))

    {

        perror("pthread_create() error");

        exit(0);

    }



    /*发送消息给客户端*/

    while(1)

    {

        bzero(se_mes,sizeof(se_mes));

        getMessage(se_mes, BUFFER_SIZE, stdin);

        sendto(sockfd,se_mes,BUFFER_SIZE,0,(struct sockaddr *)&client,sizeof(client));

    }


    /*关闭套接字*/

    close(sockfd);

    return 0;

}





/*自定义函数*/

void * function(void *arg)

{

    /*定义变量*/
  
    int sockfd,recvbytes,len;

    char re_mes[BUFFER_SIZE];

    struct ARG *info;

    info=(struct ARG *)arg;

    sockfd=info->sockfd;

    len=info->len;



    /*接收客户端发来的消息*/

    bzero(re_mes,sizeof(re_mes));

    recvbytes=recvfrom(sockfd,re_mes,BUFFER_SIZE,0,(struct sockaddr *)&client,&len);

    if(recvbytes==-1)

    {

        perror("recvfrom() error");

        exit(0);

    }

    printf("You got a message from client.\nIt's ip is %s,port is %d.\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));          //显示客户端信息

    printf("client: %s\n",re_mes);

    while(1)

    {

        bzero(re_mes,sizeof(re_mes));

        recvbytes=recvfrom(sockfd,re_mes,BUFFER_SIZE,0,(struct sockaddr *)&client,&len);

        if(recvbytes==-1)

        {

            perror("recvfrom() error");

            exit(0);

        }

        printf("客户端消息: %s/n",re_mes);



        if(!strcmp(re_mes,"bye"))

        {

            sendto(sockfd,"byebye",6,0,(struct sockaddr *)&client,len);

            break;

        }

        if(!strcmp(re_mes,"byebye"))

        {

            break;

        }

    }

    free(arg);        //释放内存空间

    close(sockfd);

    exit(0);

}
