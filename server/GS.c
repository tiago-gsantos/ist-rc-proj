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
    int invalid_req = 0;
    
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
        invalid_req = 2;
        strcpy(response, "ERR\n");
    };

    if(invalid_req != 2) {
        if(strcmp(command, "SNG") == 0) {
            if(parse_start(buffer, &player_id, &time) == 0)
                cmd_start(response, player_id, time);
            else {
                invalid_req = 1;
                strcpy(response, "RSG ERR\n");
            }
        }
        else if(strcmp(command, "TRY") == 0) {
            if(parse_try(buffer, &trial_num, c, &player_id) == 0)
                cmd_try(response, player_id, trial_num, c);
            else {
                invalid_req = 1;
                strcpy(response, "RTR ERR\n");
            }
        }
        else if(strcmp(command, "QUT")  == 0) {
            if(parse_quit_exit(buffer, &player_id) == 0)
                cmd_quit(response, player_id);
            else {
                invalid_req = 1;
                strcpy(response, "RQT ERR\n");
            }
        }
        else if(strcmp(command, "DBG")  == 0) {
            if(parse_debug(buffer, &player_id, &time, c) == 0)
                cmd_debug(response, player_id, time, c);
            else {
                invalid_req = 1;
                strcpy(response, "RDB ERR\n");
            }
        }
        else {
            invalid_req = 1;
            strcpy(response, "ERR\n");
        }
    }

    
    if(verbose) {
        if(invalid_req == 1)
            printf("The client sent an invalid request.\n");
        else if(invalid_req == 2)
            printf("The client didn't type a command.\n");
        else
            printf("Player %u sent a %s command\n", player_id, command);
    }

    if(sendto(fd_udp, response, strlen(response) * sizeof(char), 0, (struct sockaddr*)&addr, addrlen) < 0) return 1;
    
    return 0;
}


int server_tcp(int fd_tcp, int verbose) {
    char buffer[32];
    char command[4]; 
    char response[1024];
    unsigned int player_id;
    int invalid_req = 0;
    
    ssize_t bytes_read = 0;
    ssize_t total_bytes_read = 0;
    do {
        bytes_read = read(fd_tcp, buffer + total_bytes_read, 32 - total_bytes_read);
        
        if(bytes_read == -1)
            return 1;

        total_bytes_read += bytes_read;
        break;
    }
    while(bytes_read > 0);

    buffer[total_bytes_read] = '\0';


    if(sscanf(buffer, "%3s", command) != 1) {
        invalid_req = 2;
        strcpy(response, "ERR\n");
    };


    if(invalid_req != 2) {
        if(strcmp(command, "STR") == 0) {
            if(parse_st(buffer, &player_id) == 0)
                cmd_st(response, player_id);
            else {
                invalid_req = 1;
                strcpy(response, "RSG ERR\n");
            }
        }
        else if(strcmp(command, "SSB") == 0) {
            if(parse_sb(buffer) == 0)
                cmd_sb(response);
            else {
                invalid_req = 1;
                strcpy(response, "RTR ERR\n");
            }
        }
        else {
            invalid_req = 1;
            strcpy(response, "ERR\n");
        }
    }
    
    if(verbose) {
        if(invalid_req == 1)
            printf("The client sent an invalid request.\n");
        else if(invalid_req == 2)
            printf("The client didn't type a command.\n");
        else
            printf("Player %u sent a %s command\n", player_id, command);
    }
    
    
    ssize_t bytes_left = strlen(response) * sizeof(char);
    ssize_t bytes_written = 0;
    ssize_t total_bytes_written = 0;


    while(bytes_left > 0) {
        bytes_written = write(fd_tcp, response + total_bytes_written, bytes_left);
        if(bytes_written < 0) return 1; /*error*/
        
        bytes_left -= bytes_written;
        total_bytes_read += bytes_written;
    }
    
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

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    int optval = 1;
    
    if (setsockopt(fd_tcp, SOL_SOCKET, SO_RCVBUF, &tv, sizeof(tv)) < 0 ||
        setsockopt(fd_tcp, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        fprintf(stderr, "setsockopt failed\n");
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
                struct sockaddr_in addr;
                socklen_t addrlen = sizeof(addr);

                int new_fd = accept(fd_tcp, (struct sockaddr*) &addr, &addrlen);
                if(new_fd == -1) exit(1); // error


                char host[NI_MAXHOST], port[NI_MAXSERV];
                if(verbose) {
                    if(getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof host, port, sizeof port, 0) == 0) {
                        printf("Request sent by [%s:%s]\n", host, port);
                    }
                    else {
                        fprintf(stderr, "ERROR\n");
                    }
                }

                pid_t pid;
                if((pid = fork()) == -1)
                    exit(1); // error
                else if (pid == 0) {
                    close(fd_tcp);
                    
                    server_tcp(new_fd, verbose);

                    close(new_fd);

                    exit(0);
                }

                int ret;
                do ret = close(new_fd);
                while(ret == -1);
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

