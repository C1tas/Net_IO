//SERVER
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>//use select() for multiplexing
#include <sys/fcntl.h> // for non-blocking
#define MAX_LENGTH 1024
/* Select() params
 * int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
 * FD_SET(int fd, fd_set *set);
 * FD_CLR(int fd, fd_set *set);
 * FD_ISSET(int fd, fd_set *set);
 * FD_ZERO(fd_set *set);
 */
void error(char *message)
{
    perror(message);
    exit(1);
}
int main()
{
    fd_set original_socket;
    fd_set original_stdin;
    fd_set readfds;
    fd_set writefds;
    struct timeval tv;
    int numfd;
    int socket_fd, bytes_read;
    unsigned int address_length;
    char recieve_data[MAX_LENGTH],send_data[MAX_LENGTH];
    struct sockaddr_in server_address , client_address;
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
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
    // since we got s2 second, it's the "greater", so we use that for
    // the n param in select()
    numfd = socket_fd + 1;
    // wait until either socket has data ready to be recv()d (timeout 10.5 secs)
    tv.tv_sec = 10;
    tv.tv_usec = 500000;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(5000);
    server_address.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_address.sin_zero),8);
    if (bind(socket_fd,(struct sockaddr *)&server_address, sizeof(struct sockaddr)) == -1)
    {
        error("bind()");
    }
    address_length = sizeof(struct sockaddr);
    printf("\nUDP_Server Waiting for client to respond...\n");
    fflush(stdout);
    printf("Type (q or Q) at anytime to quit\n");
    while (1)
    {
        readfds = original_socket;
        writefds = original_stdin;//problem
        int recieve = select(numfd, &readfds, &writefds,/*NULL,*/ NULL, &tv);
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
                FD_CLR(socket_fd, &readfds);
                bytes_read = recvfrom(socket_fd,recieve_data,MAX_LENGTH,0,(struct sockaddr *)&client_address, &address_length); //block call, will wait till client enters something, before proceeding
                recieve_data[bytes_read - 1] = '\0'; //add null to the end of the buffer
                if((strcmp(recieve_data , "q") == 0) || (strcmp(recieve_data , "Q") == 0)) { //if client quit, then quit also
                    printf("\nClient has exited the chat.\n");
                    break;
                }
                printf("\n(%s , %d) said: %s\n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port),recieve_data);
            }
            else if (FD_ISSET(/*socket_fd*/0, &writefds)) //if set to write
                //else
            {
                FD_CLR(0, &writefds);
                printf("SERVER: ");
                fgets (send_data, MAX_LENGTH, stdin); //input the name with a size limit of MAX_LENGTH
                if ((strlen(send_data)>0) && (send_data[strlen (send_data) - 1] == '\n')) //if there is a trailing \n, remove it
                {
                    send_data[strlen (send_data) - 1] = '\0';
                }
                if ((strcmp(send_data , "q") == 0) || (strcmp(send_data , "Q") == 0)) { //if user enters q, then quit
                    sendto(socket_fd,send_data,strlen(send_data),0,(struct sockaddr *)&client_address, sizeof(struct sockaddr));
                    break;
                }
                sendto(socket_fd,send_data,strlen(send_data),0,(struct sockaddr *)&client_address, sizeof(struct sockaddr));
                fflush(stdout);
            }
            else printf("\nOOPS! What happened? SERVER");
        } //end else
    }//end while
    close (socket_fd);
    return 0;
}
