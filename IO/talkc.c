//CLIENT
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> //host struct
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>//use select() for multiplexing
#include <sys/fcntl.h> // for non-blocking
#define MAX_LENGTH 1024
#define HOST "localhost" //127.0.0.1
void error(char *message)
{
    perror(message);
    exit(1);
}
int main(int argc, char *argv[])
{
    printf("argv[0] = %s, argv[1] = %s, argv[2] = %s, argv[3] = %s\n",argv[0], argv[1], argv[2], argv[3] );
    int var2 = atoi(argv[2]);
    int var3 = atoi(argv[3]);
    printf("var2 = %d, var3 = %d\n", var2, var3);
    fd_set original_socket;
    fd_set original_stdin;
    fd_set readfds;
    fd_set writefds;
    struct timeval tv;
    int numfd;
    int socket_fd,bytes_recieved, from_fd;
    unsigned int address_length;
    struct sockaddr_in server_address, addr_from;
    struct hostent *host;
    char send_data[MAX_LENGTH],recieve_data[MAX_LENGTH];
    host = (struct hostent *) gethostbyname((char *)HOST);//127.0.0.1
    if(argc != 4){
      printf("Usage: %s des_IP des_port orgin_port \n", argv[0]);
      exit(1);
    }
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        error("socket()");
    }
    if ((from_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        error("socket()");
    }
    int flags = fcntl(socket_fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(socket_fd, F_SETFL, flags);
    //fcntl(socket_fd, F_SETFL, O_NONBLOCK); //set socket to non-blocking
    // clear the set ahead of time
    FD_ZERO(&original_socket);
    FD_ZERO(&original_stdin);
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    // add our descriptors to the set (0 - stands for STDIN)
    FD_SET(socket_fd, &original_socket);//instead of 0 put socket_fd
    FD_SET(socket_fd, &readfds);
    FD_SET(0,&original_stdin);
    FD_SET(0, &writefds);
    FD_SET(from_fd, &)
    // since we got s2 second, it's the "greater", so we use that for
    // the n param in select()
    numfd = socket_fd + 1;
    // wait until either socket has data ready to be recv()d (timeout 10.5 secs)
    tv.tv_sec = 10;
    tv.tv_usec = 500000;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    server_address.sin_addr = *((struct in_addr *)host->h_addr);

    addr_from.sin_family=AF_INET;
    addr_from.sin_port=htons(atoi(argv[3]));//獲得任意空閑端口
    addr_from.sin_addr.s_addr=htons(INADDR_ANY);//獲得本機地址


    bzero(&(server_address.sin_zero),8);
    bzero(&(addr_from.sin_zero),8);
    int opt = SO_REUSEADDR;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt,sizeof(opt));
    setsockopt(from_fd, SOL_SOCKET, SO_REUSEADDR, &opt,sizeof(opt));
    if (bind(socket_fd,(struct sockaddr *)&server_address, sizeof(struct sockaddr)) == -1)
    {
        error("bind()");
    }
    if (bind(from_fd,(struct sockaddr *)&addr_from, sizeof(struct sockaddr)) == -1)
    {
        error("bind()");
    }
    address_length = sizeof(struct sockaddr);
    printf("\nWaiting for connect...\n");
    fflush(stdout);

    printf("Type (q or Q) at anytime to quit\n");

    while (1)
    {
        readfds = original_socket;
        writefds = original_stdin;//problem
        int recieve = select(numfd, &readfds, /*NULL*/&writefds, NULL, &tv);
        if (recieve == -1)
        {
            perror("select"); // error occurred in select()
        }
        else if (recieve == 0)
        {
            printf("Timeout occurred!  No data after 10.5 seconds.\n");
        }
        else
        {
            // one or both of the descriptors have data
            if (FD_ISSET(socket_fd, &readfds)) //if set to read
            {
                FD_CLR(socket_fd, &readfds);//clear the set
                bytes_recieved = recvfrom(socket_fd,recieve_data, sizeof(recieve_data),0,(struct sockaddr *)&server_address,&address_length);
                recieve_data[bytes_recieved]= '\0';
                if((strcmp(recieve_data , "q") == 0) || (strcmp(recieve_data , "Q") == 0)) //if client quit, then quit also
                {
                    printf("\nServer has exited the chat.\n");
                    break;
                }
                printf("\n(%s , %d) said: %s\n",inet_ntoa(server_address.sin_addr),ntohs(server_address.sin_port),recieve_data);
                //inet_ntoa returns an ip address ipv4 style, ex: 127.0.0.1, and ntohs returns the port in the converted byte ordering
            }
            else if (FD_ISSET(0/*socket_fd*/, &writefds)) //if set to write
                //else
            {
                FD_CLR(0, &writefds);
                printf("ClIENT: ");
                fgets (send_data, MAX_LENGTH, stdin);
                if ((strlen(send_data)>0) && (send_data[strlen (send_data) - 1] == '\n')) //remove trailing newline, if exists
                {
                    send_data[strlen (send_data) - 1] = '\0';
                }
                if ((strcmp(send_data , "q") == 0) || strcmp(send_data , "Q") == 0) //if user quits, then send an invisible message to server to quit also
                {
                    sendto(from_fd, send_data, strlen(send_data), 0, (struct sockaddr *)&addr_from, sizeof(struct sockaddr));
                    break;
                }
                else
                {
                    sendto(from_fd, send_data, strlen(send_data), 0, (struct sockaddr *)&addr_from, sizeof(struct sockaddr));
                }
                fflush(stdout);
            }
            else printf("\nOOPS! What happened? CLIENT");
        } //end else
    } // end while
    close (socket_fd);
}
