#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
extern int errno;

#define GROUP_NUM 91
#define PORT_STRLEN 6



void getMyIP(char *ip_addr){
    struct addrinfo hints,*res;
    struct in_addr *addr;
    int errcode;


    char host_name[128];
    if(gethostname(host_name,128)==-1) {
        fprintf(stderr,"Error: %s\n", strerror(errno));
        exit(1);
    }


    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_CANONNAME;
    
    if((errcode = getaddrinfo(host_name,NULL,&hints,&res)) != 0) {
        fprintf(stderr, "Error: getaddrinfo: %s\n", gai_strerror(errcode));
        exit(1);
    }

    
    addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
    inet_ntop(res->ai_family, addr, ip_addr, INET_ADDRSTRLEN);
    
    freeaddrinfo(res);

    return;
}


int main(int argc, char *argv[]) {
    char GSIP[INET_ADDRSTRLEN];
    char *GSport;

    char defaultPort[PORT_STRLEN];
    sprintf(defaultPort, "%d", 58000 + GROUP_NUM);
    
    switch (argc) {
        case 1:
            getMyIP(GSIP);
            GSport = defaultPort;
            break;
        case 3:
            if(strcmp(argv[1], "-n") == 0) {
                strcpy(GSIP, argv[2]);
                GSport = defaultPort;
            }
            else if(strcmp(argv[1], "-p") == 0){
                getMyIP(GSIP);
                GSport = argv[2];
            }
            else {
                fprintf(stderr, "Invalid format. Please use: ./player [-n GSIP] [-p GSport]\n");
                return 1;
            }
            break;
        case 5:
            if(strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-p") == 0){
                strcpy(GSIP, argv[2]);
                GSport = argv[4];
            }
            else if(strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-n") == 0){
                strcpy(GSIP, argv[4]);
                GSport = argv[2];
            }
            else{
                fprintf(stderr, "Invalid format. Please use: ./player [-n GSIP] [-p GSport]\n");
                return 1;
            }
            break;
        default:
            fprintf(stderr, "Invalid number of parameters. Please use: ./player [-n GSIP] [-p GSport]\n");
            return 1;
    }

    printf("IP: %s\nPort: %s\n", GSIP, GSport);

    return 0;
}