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
#include "commands.h"

#define DEFAULT_PORT "58091"
#define PORT_STRLEN 6



void get_addr_info(char *GSIP, char* GSport, struct addrinfo **res) {
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
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_CANONNAME;
    
    if((errcode = getaddrinfo(GSIP, GSport, &hints, res)) != 0) {
        fprintf(stderr, "Error: getaddrinfo: %s\n", gai_strerror(errcode));
        exit(1);
    }

    return;
}


int play(struct addrinfo *res, int fd_udp) {
    int trial_num = -1;
    unsigned int player_id = 0;
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
            if(parse_start(buffer, request, &player_id, trial_num) == 0)
                cmd_start(request, &player_id, &trial_num, fd_udp, res);
        } 
        else if(strcmp(command, "try") == 0) {
            if(parse_try(buffer, request, player_id, trial_num) == 0)
                printf("%s", request);
            //cmd_try();
        }
        else if(strcmp(command, "show_trials") == 0 || strcmp(command, "st") == 0) {
            if(parse_st(buffer, request, player_id) == 0) 
                printf("%s", request);
            //cmd_st();
        }
        else if(strcmp(command, "scoreboard") == 0 || strcmp(command, "sb") == 0) {
            if(parse_sb(buffer, request) == 0)
                printf("%s", request);
            //cmd_sb();
        }
        else if(strcmp(command, "quit")  == 0) {
            if(parse_quit_exit(buffer, request, player_id, trial_num) == 0)
                printf("%s", request);
            //cmd_quit();
        }
        else if(strcmp(command, "exit")  == 0) {
            if(parse_quit_exit(buffer, request, player_id, trial_num) == 1)
                printf("%s", request);
            //cmd_exit();
        }
        else if(strcmp(command, "debug")  == 0) {
            if(parse_debug(buffer, request, &player_id, trial_num) == 0)
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
    char GSIP[INET_ADDRSTRLEN] = "";
    char GSport[PORT_STRLEN] = DEFAULT_PORT;
    struct addrinfo *res;
    
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

    //printf("IP: %s\nPort: %s\n", GSIP, GSport);
    get_addr_info(GSIP, GSport, &res);

    int fd_udp = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if(fd_udp == -1) exit(1); /* error */

    int response = play(res, fd_udp);
    return response;
}