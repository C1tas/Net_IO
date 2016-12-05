#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, const char** argv){
    struct hostent * hp;
    char **p;
    if (argc != 3){
        printf("Usage: %s hostname.\n", argv[0]);
        exit(1);
    }
    if (strncmp(argv[2], "addr", 4) == 0){
        hp = gethostbyname(argv[1]);
        if(hp == NULL){
            printf("Host infomation for %s is not found.\n", argv[1]);
            exit(2);
        }

        for (p = hp->h_addr_list; *p != 0; p++){
            struct in_addr in;
            char **q;
            memcpy(&in.s_addr, *p, sizeof(in.s_addr));
            printf("%s\t%s", inet_ntoa(in), hp->h_name);
            for (q = hp->h_aliases; *q != 0; q++){
                printf("%s\n.", *q);
            }
        }

    }
    else {
        ulong addr;
        if((int)(addr = inet_addr(argv[1])) == -1){
            printf("IP-addr must be of the form a.b.c\n");
            exit(2);
        }
        hp = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
        if(hp == NULL){
            printf("Host infomation for %s is not found.\n", argv[1]);
            exit(3);
        }
        for (p = hp->h_addr_list; *p != 0; p++){
            struct in_addr in;
            char **q;
            memcpy(&in.s_addr, *p, sizeof(in.s_addr));
            printf("%s\t%s", inet_ntoa(in), hp->h_name);
            for (q = hp->h_aliases; *q != 0; q++){
                printf("%s.\n", *q);
            }
        }
    }
    return 0;
}
