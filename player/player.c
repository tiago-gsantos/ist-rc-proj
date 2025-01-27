#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
extern int errno;

#include "parser_player.h"
#include "commands.h"

#define DEFAULT_PORT "58091"
#define PORT_STRLEN 6
#define WAIT_TIME_LIMIT 15



void get_addr_info(char *GSIP, char* GSport, struct addrinfo **res_udp, struct addrinfo **res_tcp) {
    struct addrinfo hints;
    int errcode;

    if(strcmp(GSIP, "") == 0) {
        if(gethostname(GSIP, INET_ADDRSTRLEN) == -1) {
            fprintf(stderr,"Error: %s\n", strerror(errno));
            exit(1);
        }
    }


    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_CANONNAME;
    
    if((errcode = getaddrinfo(GSIP, GSport, &hints, res_udp)) != 0) {
        fprintf(stderr, "Error: getaddrinfo: %s\n", gai_strerror(errcode));
        exit(1);
    }


    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_CANONNAME;
    
    if((errcode = getaddrinfo(GSIP, GSport, &hints, res_tcp)) != 0) {
        fprintf(stderr, "Error: getaddrinfo: %s\n", gai_strerror(errcode));
        freeaddrinfo(*res_udp);
        exit(1);
    }

    return;
}


int play(struct addrinfo *res_udp, struct addrinfo *res_tcp, int fd_udp) {
    int trial_num = -1;
    unsigned int player_id = 0;

    while(1) {
        char buffer[256];
        char command[12]; 
        char request[32];

        if(!fgets(buffer, sizeof(buffer), stdin)) {
            fprintf(stderr, "Error reading from stdin!\n");
            return 1;
        }

        if(sscanf(buffer, "%11s", command) != 1) {
            fprintf(stderr, "Please type a command!\n");
            continue;
        };
        
        // Execute command
        if(strcmp(command, "start") == 0) {
            if(parse_start(buffer, request, trial_num, &player_id) == 0)
                if(cmd_start(request, &player_id, &trial_num, fd_udp, res_udp) == 1)
                    return 1;
        }
        else if(strcmp(command, "try") == 0) {
            if(parse_try(buffer, request, player_id, trial_num) == 0)
                if(cmd_try(request, player_id, &trial_num, fd_udp, res_udp) == 1)
                    return 1;
        }
        else if(strcmp(command, "show_trials") == 0 || strcmp(command, "st") == 0) {
            if(parse_st(buffer, request, player_id) == 0) 
                if(cmd_st(request, res_tcp) == 1)
                    return 1;
        }
        else if(strcmp(command, "scoreboard") == 0 || strcmp(command, "sb") == 0) {
            if(parse_sb(buffer, request) == 0)
                if(cmd_sb(request, res_tcp) == 1)
                    return 1;
        }
        else if(strcmp(command, "quit")  == 0) {
            if(parse_quit_exit(buffer, request, player_id, trial_num) == 0)
                if(cmd_quit(request, &trial_num, fd_udp, res_udp))
                    return 1;
        }
        else if(strcmp(command, "exit")  == 0) {
            if(parse_quit_exit(buffer, request, player_id, trial_num) == 0)
                if(cmd_quit(request, &trial_num, fd_udp, res_udp) == 1)
                    return 1;
            break;
        }
        else if(strcmp(command, "debug")  == 0) {
            if(parse_debug(buffer, request, trial_num, &player_id) == 0)
                if(cmd_debug(request, &player_id, &trial_num, fd_udp, res_udp) == 1)
                    return 1;
        }
        else
            fprintf(stderr, "Invalid command!\n");
    }

    return 0;
}



int main(int argc, char *argv[]) {
    char GSIP[INET_ADDRSTRLEN] = "";
    char GSport[PORT_STRLEN] = DEFAULT_PORT;
    struct addrinfo *res_udp, *res_tcp;
    struct timeval timeout;
    
    switch (argc) {
        case 1:
            break;
        case 3:
            if(strcmp(argv[1], "-n") == 0) {
                strcpy(GSIP, argv[2]);
            }
            else if(strcmp(argv[1], "-p") == 0){
                strcpy(GSport, argv[2]);
            }
            else {
                fprintf(stderr, "Invalid format. Please use: ./player [-n GSIP] [-p GSport]\n");
                return 1;
            }
            break;
        case 5:
            if(strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-p") == 0){
                strcpy(GSIP, argv[2]);
                strcpy(GSport, argv[4]);
            }
            else if(strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-n") == 0){
                strcpy(GSIP, argv[4]);
                strcpy(GSport, argv[2]);
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

    get_addr_info(GSIP, GSport, &res_udp, &res_tcp);

    int fd_udp = socket(AF_INET, SOCK_DGRAM, 0); 
    if(fd_udp == -1) {
        fprintf(stderr, "Error in socket creation\n");
        freeaddrinfo(res_udp);
        freeaddrinfo(res_tcp);
        return 1;
    }

    timeout.tv_sec = WAIT_TIME_LIMIT;  
    timeout.tv_usec = 0;
    if (setsockopt(fd_udp, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        fprintf(stderr, "Error in socket creation\n");
        freeaddrinfo(res_udp);
        freeaddrinfo(res_tcp);
        close(fd_udp);
        return 1;
    }


    int response = play(res_udp, res_tcp, fd_udp);

    close(fd_udp);
    freeaddrinfo(res_udp);
    freeaddrinfo(res_tcp);
    
    return response;
}