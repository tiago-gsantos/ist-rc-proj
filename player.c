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

#include "parser_player.h"

#define GROUP_NUM 91
#define PORT_STRLEN 6



void get_my_IP(char *ip_addr){
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


int play(char *GSIP, char* GSport, int fd_udp) {
    while(1) {
        char buffer[256];
        char command[12]; 
        char request[256];

        if(!fgets(buffer, sizeof(buffer), stdin)) {
            fprintf(stderr, "ERROR!\n");
            return 1;
        }

        if(sscanf(buffer, "%11s", command) != 1) {
            fprintf(stderr, "Please type a command!\n");
            continue;
        };
        
        // Execute command
        if(strcmp(command, "start") == 0) {
            if(parse_start(buffer, request) == 0)
                printf("%s", request);
            //cmd_start();
        } 
        else if(strcmp(command, "try") == 0) {
            if(parse_try(buffer, request) == 0) // falta args
                printf("%s", request);
            //cmd_try();
        }
        else if(strcmp(command, "show_trials") == 0 || strcmp(command, "st") == 0) {
            if(parse_st(buffer, request) == 0) // falta args
                printf("%s", request);
            //cmd_st();
        }
        else if(strcmp(command, "scoreboard") == 0 || strcmp(command, "sb") == 0) {
            if(parse_sb(buffer, request) == 0)
                printf("%s", request);
            //cmd_sb();
        }
        else if(strcmp(command, "quit")  == 0) {
            if(parse_quit_exit(buffer, request) == 0)
                printf("%s", request);
            //cmd_quit();
        }
        else if(strcmp(command, "exit")  == 0) {
            if(parse_quit_exit(buffer, request) == 1)
                printf("%s", request);
            //cmd_exit();
        }
        else if(strcmp(command, "debug")  == 0) {
            if(parse_debug(buffer, request) == 0)
                printf("%s", request);
            //cmd_debug();
        }
        else {
            fprintf(stderr, "Invalid command!\n");
        }
    }

    return 0;
}






int main(int argc, char *argv[]) {
    char GSIP[INET_ADDRSTRLEN];
    char *GSport;

    char default_port[PORT_STRLEN];
    sprintf(default_port, "%d", 58000 + GROUP_NUM);
    
    switch (argc) {
        case 1:
            get_my_IP(GSIP);
            GSport = default_port;
            break;
        case 3:
            if(strcmp(argv[1], "-n") == 0) {
                strcpy(GSIP, argv[2]);
                GSport = default_port;
            }
            else if(strcmp(argv[1], "-p") == 0){
                get_my_IP(GSIP);
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

    int fd_udp = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
    if(fd_udp == -1) exit(1); /* error */

    int response = play(GSIP, GSport, fd_udp);
    return response;
}