
/*
 ============================================================================
 Name        : Server.c
 Author      : Tony
 Version     :
 Copyright   : Your copyright notice
 Description : Socket UDP 协议的服务器端
 ============================================================================
 */

/*
通常所见的的TCP服务器都是并发实现的，即服务同时处理多个请求，
而不是等待前一个完成再处理下一个请求，这个实现得益于TCP的listen()
与connect()的分工处理机制。具体为，服务器监听来自客户的连接，当一个请求到来时，
服务器fork()一个子进程，处理该请求，然后父进程继续监听外部请求。
但在UDP中，没有这种监听和连接机制，所以它必须等待前一处理完成才能继续处理下一个客户的请求。
但并不是说UDP实现并发服务器是不可能的，只是与上面的实现稍有不同。原理如下：服务器（知名端口）
等待一下客户的到来，当一个客户到来后，记下其IP和port，然后同理，服务器fork一个子进程，
建立一个socket再bind一个随机端口，然后建立与客户的连接，并处理该客户的请求。父进程继续循环，
等待下一个客户的到来。在tftpd中就是使用这种技术的。

*/



 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>          
#include <sys/socket.h>     
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/*
 * 操作流程
 * socket
 * bind
 * sendto/recvform
*/
#define SERVER_PORT 8888  //server端口号


int main(int argc, char **argv)
{
    //int iPort;
    int iRst;
    int iRecvLen;
    int iRecvRandomLen;/* 子进程中随机端口接收到的数据 */
    unsigned char ucRecvBuf[1000];
    unsigned char ucRecvRandomBuf[1000];/* 子进程中随机端口使用的接收buf */
    //iPort = SERVER_PORT;
    
    /*
     struct sockaddr_in{
                unsigned short          sin_family;     
                unsigned short int      sin_port;
                struct in_addr          sin_addr;
                unsigned char           sin_zero[8];
        }
    */
    struct sockaddr_in tSocketServerAddr;
    struct sockaddr_in tSocketClientAddr;
    socklen_t tSocketClientAddrLen;
    int iSocketServerFB;
    int iSocketServerRandomFB;/* 需要一个随即端口，用来实现UDP多进程并发处理使用 */

    /*
     *  int socket(int domain, int type,int protocol)
     *  domain:说明我们网络程序所在的主机采用的通讯协族(AF_UNIX和AF_INET等).
     *         AF_UNIX只能够用于单一的Unix 系统进程间通信,
     *         而AF_INET是针对Internet的,因而可以允许在远程
     *  type:我们网络程序所采用的通讯协议(SOCK_STREAM,SOCK_DGRAM等)
     *       SOCK_STREAM表明我们用的是TCP 协议,这样会提供按顺序的,可靠,双向,面向连接的比特流.
     *       SOCK_DGRAM 表明我们用的是UDP协议,这样只会提供定长的,不可靠,无连接的通信.
     *  protocol:由于我们指定了type,所以这个地方我们一般只要用0来代替就可以了
     *  socket为网络通讯做基本的准备.成功时返回文件描述符,失败时返回-1,看errno可知道出错的详细情况.
    */
    iSocketServerFB = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == iSocketServerFB)
    {
        printf("socket error !\n");
        return -1;
    }
    
    /*
     struct sockaddr_in{
                unsigned short          sin_family;     
                unsigned short int      sin_port;
                struct in_addr          sin_addr;
                unsigned char           sin_zero[8];
        }
    */
    
   /*
    * 不过由于系统的兼容性,我们一般不用这个头文件,而使用另外一个结构(struct sockaddr_in) 
    * 来代替.在中有sockaddr_in的定义 
   */
    memset(&tSocketServerAddr, 0, sizeof(struct sockaddr));
    tSocketServerAddr.sin_family      = AF_INET;//一般为AF_INET,针对inter net
    /*htons: 将本机上的long数据转化成网络上的short型数据*/
    tSocketServerAddr.sin_port        = htons(SERVER_PORT);//host to net ,short 要监听的端口
    tSocketServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);//表示可以和任何的主机通信
    memset(tSocketServerAddr.sin_zero, 0, 8);
    /*
     *  int bind(int sockfd, struct sockaddr *my_addr, int addrlen)
    */
    iRst = bind(iSocketServerFB, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
    if (-1 == iRst)
    {
        printf("bind error\n!");
    }

    while(1)
    {
        /*
         * int recvfrom(int sockfd,void *buf,int len,
         *              unsigned int flags,struct sockaddr * from ,socklen_t *fromlen)
         * sockfd,buf,len的意义和read,write一样,分别表示套接字描述符,发送或接收的缓冲区及大小.
         * recvfrom负责从 sockfd接收数据,如果from不是NULL,
         * 那么在from里面存储了信息来源的情况,如果对信息的来源不感兴趣,
         * 可以将from和fromlen 设置为NULL.               
        */
        tSocketClientAddrLen = sizeof(struct sockaddr);//这个长度必须初始化，否则第一次返回来的ip地址不对
        iRecvLen = recvfrom(iSocketServerFB, ucRecvBuf, 
                            999, 0, (struct sockaddr *)&tSocketClientAddr, &tSocketClientAddrLen);
        if (iRecvLen < 0)
        {
            printf("receive from client error\n");
            return -1;
        }
        printf("RecvSocketAddr ip %s: %d ", inet_ntoa(tSocketClientAddr.sin_addr), ntohs(tSocketClientAddr.sin_port));
        ucRecvBuf[iRecvLen] = '\0';
        printf("ucRecvBuf = %s\n", ucRecvBuf);

        
 
        /*
        * pid_t fork(void)
        * 功能: 创建子进程
        * fork的奇妙之处在于它被调用一次，却返回2次，他可能有3种不同的返回值
        * a.在父进程中，fork返回新创建的子进程的PID
        * b.在子进程中，fork返回0
        * c.如果出现错误，fork返回一个负值
        * 注意:fork创建的子进程，代码共享，但是数据不共享
        */
        //iPort++;/* UDP随机端口 */
#if 1        
        if (fork() == 0)
        {
            /* 子进程需要跳出这个while循环，while后面的代码处理客户端进来的数据 */
            //printf("break while \n");
            break;
        }
#else
        iRst = sendto(iSocketServerFB, "world", sizeof("world"), 0, 
                       (struct sockaddr *)&tSocketClientAddr, sizeof(struct sockaddr));

#endif        
        /* 这里继续主进程的while循环，用来处理接收新的客户端传进来的数据 */
        printf("next progress================\n");
    }
    /* 主进程运行不到这里，这里是子进程的代码
     * 原理如下：服务器（知名端口）
     * 等待一下客户的到来，当一个客户到来后，记下其IP和port，然后同理，服务器fork一个子进程，
     * 建立一个socket再bind一个随机端口，然后建立与客户的连接，并处理该客户的请求。父进程继续循环，
     * 等待下一个客户的到来。在tftpd中就是使用这种技术的。
    */
    printf("enter fork\n");
    iSocketServerRandomFB = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSocketServerRandomFB == -1)
    {
        printf("Random socket error\n");
        return -1;
    }
        
    /*
     * sendto负责向to发送信息.此时在to里面存储了收信息方的详细资料.
     * int sendto(int sockfd,const void *msg,int len,unsigned int flags,struct sockaddr *to, int tolen)
     * 参数msg指向欲连线的数据内容，参数flags 一般设0，详细描述请参考send()。
     * 参数to用来指定欲传送的网络地址，结构sockaddr请参考bind()。参数tolen为sockaddr的结构长度
    */
#if 1
    printf("RecvSocketAddr ip %s: %d ", inet_ntoa(tSocketClientAddr.sin_addr), ntohs(tSocketClientAddr.sin_port));

    iRst = sendto(iSocketServerRandomFB, "world", 5, 0, 
                   (struct sockaddr *)&tSocketClientAddr, sizeof(struct sockaddr));
#else
    iRst = sendto(iSocketServerFB, "world", 5, 0, 
                   (struct sockaddr *)&tSocketClientAddr, sizeof(struct sockaddr));

#endif
    if (iRst == -1)
    {
        printf("random send error\n");
        return -1;
    }
    else
    {
        //printf("random send ok\n");
    }
    while(1)
    {
        iRecvRandomLen = recvfrom(iSocketServerRandomFB, ucRecvRandomBuf, 999, 0, 
                                   (struct sockaddr *)&tSocketClientAddr,
                                   &tSocketClientAddrLen);
        unsigned char tmp[1000];
        int i = 0;
        for(i = 0; i < iRecvRandomLen - 1; i++){
          tmp[iRecvRandomLen - 1 - i] = ucRecvRandomBuf[i];
        }
        tmp[iRecvRandomLen] = '\0';
        printf("tmp = %s\n", tmp);
        sendto(iSocketServerFB, tmp, 999, 0, (struct sockaddr*)&tSocketClientAddr, sizeof(struct sockaddr));
        if (iRecvRandomLen < 0)
        {
            printf("random recvice error\n");
            return -1;
        }
        ucRecvRandomBuf[iRecvRandomLen] = '\0';
        printf("ip:%s:%d random recvice buf = %s",
                    inet_ntoa(tSocketClientAddr.sin_addr), 
                    ntohs(tSocketClientAddr.sin_port),
                    ucRecvRandomBuf);
    }
    close(iSocketServerFB);
    return 0;
}
