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

#define SERVER_PORT 8888  //server端口号

/* socket
 * connect可以没有
 * sendto/recvfrom
 */
int main(int argc,char **argv)
{
    int i;
    int iFormatLen;
    int iSocketClientFb;
    int iRst;
    int iRecvSocketAddrLen;
    unsigned char ucRecvBuf[1000];
    unsigned char ucSendBuf[1000]; 
    struct sockaddr_in tSocketServerAddr;
    struct sockaddr_in tRecvSocketAddr;
    struct sockaddr_in netaddr;
    if (argc != 2)
    {
        printf("Usage:\n");
        printf("%s <server_ip>\n", argv[0]);
        return -1;
    }
#if 0
    memset(&netaddr, 0, sizeof(struct sockaddr));
    netaddr.sin_family = AF_INET;
    netaddr.sin_port = htons(9999);
    netaddr.sin_addr.s_addr = INADDR_ANY;
#endif
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
    iSocketClientFb = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == iSocketClientFb)
    {
        printf("socket error !\n");
        return -1;
    }
#if 0
    if (bind(iSocketClientFb, (struct sockaddr*)&netaddr, sizeof(struct sockaddr)) == -1)
    {
        printf("bind error\n");
        exit(1);
        
    }
#endif    
    memset(&tSocketServerAddr, 0, sizeof(struct sockaddr));
    tSocketServerAddr.sin_family = AF_INET;//inter net 使用这个参数
    tSocketServerAddr.sin_port   = htons(SERVER_PORT);/* host to net short */
    /*
     * int inet_aton(const char *cp,struct in_addr *inp)
     * char *inet_ntoa(struct in_addr in)
     * 字符串的IP和32位的IP转换. 
     * 在网络上面我们用的IP都是数字加点(192.168.0.1)构成的, 
     * 而在struct in_addr结构中用的是32位的IP, 
     * 我们上面那个32位IP(C0A80001)是的192.168.0.1 为了转换我们可以使用下面两个函数
     * 函数里面 a 代表 ascii n 代表network.第一个函数表示将a.b.c.d的IP转换为32位的IP,
     * 存储在 inp指针里面.第二个是将32位IP转换为a.b.c.d的格式.
    */
    if (0 == inet_aton(argv[1], &tSocketServerAddr.sin_addr))
    {
        printf("invalid server_ip\n");
        return -1;
    }
    memset(tSocketServerAddr.sin_zero, 0, 8);
    /*
     * int sendto ( socket s , const void * msg, int len, unsigned int flags, const   
     *               struct sockaddr * to , int tolen ) ;
     * sendto() 用来将数据由指定的socket传给对方主机。
     * 参数s为已建好连线的socket,如果利用UDP协议则不需经过连线操作。
     * 参数msg指向欲连线的数据内容，参数flags 一般设0，详细描述请参考send()。
     * 参数to用来指定欲传送的网络地址，结构sockaddr请参考bind()。参数tolen为sockaddr的结构长度
    */

#if 0
    while(1)
    {
        if (fgets(ucSenddBuf, 999, stdin))
        {
            sendto(iSocketClientFb, ucSenddBuf, strlen(ucSenddBuf), 0, 
                      (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
        }
    }
#else
    iRst = sendto(iSocketClientFb, "hello", sizeof("hello"), 0, (const struct sockaddr *)&tSocketServerAddr,
                  sizeof(struct sockaddr));
    if (iRst == -1)
    {
        printf("sendto error!!!! \n");
        return -1;
    }
    iRecvSocketAddrLen = sizeof(struct sockaddr);
    iRst = recvfrom(iSocketClientFb, ucRecvBuf, 999, 0, (struct sockaddr *)&tRecvSocketAddr, &iRecvSocketAddrLen);
    //iRst = recvfrom(iSocketClientFb, ucRecvBuf, 999, 0, NULL, NULL);

    if (iRst == -1)
    {
        printf("recvfrom error!!!! \n");
        return -1;
    }

    /* 
     * inet_ntoa 将这个结构体中的ip地址格式化成.192.168.x.x样式进行打印输出
     * ntohs 将网络字节序转化为本机字节序
     */
    ucRecvBuf[iRst] = '\0';
    printf("RecvSocketAddr ip %s: %d ", inet_ntoa(tRecvSocketAddr.sin_addr), ntohs(tRecvSocketAddr.sin_port));
    printf("buf = %s\n",ucRecvBuf);

    for(i = 0; i < 20; i++)
    {
        iFormatLen = sprintf(ucSendBuf, "shit %d \n", i);
        iRst = sendto(iSocketClientFb, ucSendBuf, iFormatLen, 0, (struct sockaddr *)&tRecvSocketAddr, sizeof(struct sockaddr));
        unsigned char tmp[1000];
        int iR;
        iR = recvfrom(iSocketClientFb, tmp, 999, 0, (struct sockaddr*)&tRecvSocketAddr, &iRecvSocketAddrLen);
        tmp[iR] = '\0';
        printf("buf = %s.\n", tmp);
        if (iRst == -1)
        {
            printf("%d sendto error!!!! \n",__LINE__);
            return -1;
        }
        printf("send ok\n");
        sleep(1);
    }
#endif    
    close(iSocketClientFb);
    return 0;
}
