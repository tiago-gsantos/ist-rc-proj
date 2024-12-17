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

#include "commands.h"
#include "parser_server.h"

#define DEFAULT_PORT "58091"
#define PORT_STRLEN 6




void get_addr_info(char* GSport, struct addrinfo **res_udp, struct addrinfo **res_tcp) {
    struct addrinfo hints;
    int errcode;

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype=SOCK_DGRAM; // UDP
    hints.ai_flags=AI_PASSIVE;
    
    if((errcode = getaddrinfo(NULL, GSport, &hints, res_udp)) != 0) {
        fprintf(stderr, "Error: getaddrinfo: %s\n", gai_strerror(errcode));
        exit(1);
    }


    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype=SOCK_STREAM; // TCP
    hints.ai_flags=AI_PASSIVE;
    
    if((errcode = getaddrinfo(NULL, GSport, &hints, res_tcp)) != 0) {
        fprintf(stderr, "Error: getaddrinfo: %s\n", gai_strerror(errcode));
        exit(1);
    }

    return;
}

int server_udp(int fd_udp, int verbose) {
    char buffer[256];
    char command[4]; 
    char response[256];
    char c[4];
    char host[NI_MAXHOST], port[NI_MAXSERV];
    unsigned int time;
    unsigned int player_id;
    int trial_num;
    
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    
    ssize_t bytes_received = recvfrom(fd_udp, buffer, 256, 0, (struct sockaddr*)&addr, &addrlen);
    if(bytes_received == -1) {
        fprintf(stderr, "ERROR");
        return 1;
    }
    buffer[bytes_received] = '\0';

    if(verbose) {
        if(getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof host, port, sizeof port, 0) == 0) {
            printf("Request sent by [%s:%s]\n", host, port);
        }
        else
            fprintf(stderr, "ERROR");
    }

    if(sscanf(buffer, "%3s", command) != 1) {
        if(verbose)
            fprintf(stderr, "The client didn't type a command.\n");
        strcpy(response, "ERR\n");
        
        sendto(fd_udp, response, strlen(response) * sizeof(char), 0, (struct sockaddr*)&addr, addrlen);
        return 1;
    };

    if(strcmp(command, "SNG") == 0) {
        if(parse_start(buffer, &player_id, &time) == 0)
            cmd_start(response, player_id, time);
        else {
            printf("parse\n");
            strcpy(response, "RSG ERR\n");
        }
    }
    else if(strcmp(command, "TRY") == 0) {
        if(parse_try(buffer, &trial_num, c, &player_id) == 0)
            cmd_try(response, player_id, trial_num, c);
        else
            strcpy(response, "RTR ERR\n");
    }
    else if(strcmp(command, "QUT")  == 0) {
        if(parse_quit_exit(buffer, &player_id) == 0)
            cmd_quit(response, player_id);
        else
            strcpy(response, "RQT ERR\n");
    }
    else if(strcmp(command, "DBG")  == 0) {
        if(parse_debug(buffer, &player_id, &time, c) == 0)
            cmd_debug(response, player_id, time, c);
        else
            strcpy(response, "RDB ERR\n");
    }
    else {
        if(verbose)
            fprintf(stderr, "The client sent an invalid request.\n");
        strcpy(response, "ERR\n");
    }

    if(sendto(fd_udp, response, strlen(response) * sizeof(char), 0, (struct sockaddr*)&addr, addrlen) < 0) return 1;
    
    return 0;
}

int main(int argc, char *argv[]) {
    char GSport[PORT_STRLEN] = DEFAULT_PORT;
    unsigned int verbose = 0;
    struct addrinfo *res_udp, *res_tcp;
    fd_set inputs, test_fds;

    int out_fds;
    
    switch (argc) {
        case 1:
            break;
        case 2:
            if(strcmp(argv[1], "-v") == 0)
                verbose = 1;
            else {
                fprintf(stderr, "Invalid format. Please use: ./GS [-p GSport] [-v]\n");
                return 1;
            }
            break;
        case 3:
            if(strcmp(argv[1], "-p") == 0)
                strcpy(GSport, argv[2]);
            else {
                fprintf(stderr, "Invalid format. Please use: ./GS [-p GSport] [-v]\n");
                return 1;
            }
            break;
        case 4:
            if(strcmp(argv[1], "-v") == 0 && strcmp(argv[2], "-p") == 0){
                verbose = 1;
                strcpy(GSport, argv[3]);
            }
            else if(strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-v") == 0){
                verbose = 1;
                strcpy(GSport, argv[2]);
            }
            else{
                fprintf(stderr, "Invalid format. Please use: ./GS [-p GSport] [-v]\n");
                return 1;
            }
            break;
        default:
            fprintf(stderr, "Invalid number of parameters. Please use: ./GS [-p GSport] [-v]\n");
            return 1;
    }
    

    get_addr_info(GSport, &res_udp, &res_tcp);


    // UDP socket
    int fd_udp = socket(AF_INET, SOCK_DGRAM, 0); 
    if(fd_udp == -1) {
        fprintf(stderr, "Error in socket creation\n");
        return 1;
    }

    if(bind(fd_udp, res_udp->ai_addr, res_udp->ai_addrlen) == -1) {
        fprintf(stderr, "ERROR");
        return 1;
    }


    // TCP socket
    int fd_tcp = socket(AF_INET, SOCK_STREAM, 0); 
    if(fd_tcp == -1) {
        fprintf(stderr, "Error in socket creation\n");
        return 1;
    }

    if(bind(fd_tcp, res_tcp->ai_addr, res_tcp->ai_addrlen) == -1) {
        fprintf(stderr, "ERROR");
        return 1;
    }

    if(listen(fd_tcp, 1) == -1) { // numero de tcp connections ???
        fprintf(stderr, "ERROR");
        return 1;
    }

    FD_ZERO(&inputs);
    FD_SET(0, &inputs); // é preciso ligar isto ???
    FD_SET(fd_udp, &inputs);
    FD_SET(fd_tcp, &inputs);


    while(1) {
        test_fds = inputs;

        out_fds = select(FD_SETSIZE, &test_fds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);
        
        switch (out_fds)
        {
        case 0:
            printf("Timeout\n");
            break;
        case -1:
            fprintf(stderr, "ERROR");
            return 1;
        default:
            if(FD_ISSET(0, &test_fds)) {
                // input
            }

            if(FD_ISSET(fd_udp, &test_fds)) {
                server_udp(fd_udp, verbose);
            }

            if(FD_ISSET(fd_tcp, &test_fds)) {
                // accept
                // coisas tcp
            }
            break;
        }
    }

    close(fd_udp);
    close(fd_tcp);
    freeaddrinfo(res_udp);
    freeaddrinfo(res_tcp);
    
    return 0;
}

