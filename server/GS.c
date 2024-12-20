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
        freeaddrinfo(*res_udp);
        exit(1);
    }

    return;
}

int server_udp(int fd_udp, int verbose) {
    char buffer[256];
    char command[4]; 
    char response[256];
    char c[5];
    char status[6];
    char host[NI_MAXHOST], port[NI_MAXSERV];
    unsigned int time;
    unsigned int player_id;
    int trial_num;
    int invalid_req = 0;
    
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    
    ssize_t bytes_received = recvfrom(fd_udp, buffer, 256, 0, (struct sockaddr*)&addr, &addrlen);
    if(bytes_received == -1) {
        fprintf(stderr, "Error communicating with server");
        return 1;
    }
    buffer[bytes_received] = '\0';

    if(verbose) {
        if(getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof host, port, sizeof port, 0) == 0) {
            printf("Request sent by [%s:%s]\n", host, port);
        }
        else
            fprintf(stderr, "Error while getting hostname\n");
    }

    if(sscanf(buffer, "%3s", command) != 1) {
        invalid_req = 2;
        strcpy(response, "ERR\n");
    };

    if(invalid_req != 2) {
        if(strcmp(command, "SNG") == 0) {
            if(parse_start(buffer, &player_id, &time) == 0) {
                if(cmd_start(response, player_id, time) == 1)
                    return 1;
            }
            else {
                invalid_req = 1;
                strcpy(response, "RSG ERR\n");
            }
        }
        else if(strcmp(command, "TRY") == 0) {
            if(parse_try(buffer, &trial_num, c, &player_id) == 0) {
                if(cmd_try(response, player_id, trial_num, c) == 1)
                    return 1;
            }
            else {
                invalid_req = 1;
                strcpy(response, "RTR ERR\n");
            }
        }
        else if(strcmp(command, "QUT")  == 0) {
            if(parse_quit_exit(buffer, &player_id) == 0) {
                if(cmd_quit(response, player_id) == 1)
                    return 1;
            }
            else {
                invalid_req = 1;
                strcpy(response, "RQT ERR\n");
            }
        }
        else if(strcmp(command, "DBG")  == 0) {
            if(parse_debug(buffer, &player_id, &time, c) == 0) {
                if(cmd_debug(response, player_id, time, c) == 1)
                    return 1;
            }
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
            printf("The client sent an invalid request.\nSending 'ERR'\n\n");
        else if(invalid_req == 2)
            printf("The client didn't type a command.\nSending 'ERR'\n\n");
        else {
            sscanf(response, "%*s %s", status);
            printf("Player %u sent a %s command\nSending a response with status '%s'\n\n", player_id, command, status);
        }
    }

    if(sendto(fd_udp, response, strlen(response) * sizeof(char), 0, (struct sockaddr*)&addr, addrlen) < 0) {
        fprintf(stderr, "Error communicating with server");
        return 1;
    }
    
    return 0;
}


int server_tcp(int fd_tcp, int verbose) {
    char buffer[32];
    char command[4];
    char status[6];
    char response[1024];
    unsigned int player_id = 0;
    int invalid_req = 0;
    
    ssize_t bytes_read = 0;
    ssize_t total_bytes_read = 0;
    do {
        bytes_read = read(fd_tcp, buffer + total_bytes_read, 32 - total_bytes_read);
        
        if(bytes_read == -1) {
            fprintf(stderr, "Error communicating with server");
            return 1;
        }

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
                strcpy(response, "RST ERR\n");
            }
        }
        else if(strcmp(command, "SSB") == 0) {
            if(parse_sb(buffer) == 0)
                cmd_sb(response);
            else {
                invalid_req = 1;
                strcpy(response, "RSS ERR\n");
            }
        }
        else {
            invalid_req = 1;
            strcpy(response, "ERR\n");
        }
    }
    
    if(verbose) {
        if(invalid_req == 1)
            printf("The client sent an invalid request.\nSending 'ERR'\n\n");
        else if(invalid_req == 2)
            printf("The client didn't type a command.\nSending 'ERR'\n\n");
        else if(player_id == 0) {
            sscanf(response, "%*s %s", status);
            printf("A player sent a %s command\nSending a response with status '%s'\n\n", command, status);
        }
        else {
            sscanf(response, "%*s %s", status);
            printf("Player %u sent a %s command\nSending a response with status '%s'\n\n", player_id, command, status);
        }
    }
    

    ssize_t bytes_left = strlen(response) * sizeof(char);
    ssize_t bytes_written = 0;
    ssize_t total_bytes_written = 0;


    while(bytes_left > 0) {
        bytes_written = write(fd_tcp, response + total_bytes_written, bytes_left);
        if(bytes_written < 0) {
            fprintf(stderr, "Error communicating with server");
            return 1;
        }
        
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
        freeaddrinfo(res_udp);
        freeaddrinfo(res_tcp);
        fprintf(stderr, "Error in socket creation\n");
        return 1;
    }

    if(bind(fd_udp, res_udp->ai_addr, res_udp->ai_addrlen) == -1) {
        freeaddrinfo(res_udp);
        freeaddrinfo(res_tcp);
        close(fd_udp);
        fprintf(stderr, "Error binding socket\n");
        return 1;
    }


    // TCP socket
    int fd_tcp = socket(AF_INET, SOCK_STREAM, 0); 
    if(fd_tcp == -1) {
        freeaddrinfo(res_udp);
        freeaddrinfo(res_tcp);
        close(fd_udp);
        fprintf(stderr, "Error in socket creation\n");
        return 1;
    }


    int optval = 1;
    
    if (setsockopt(fd_tcp, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        freeaddrinfo(res_udp);
        freeaddrinfo(res_tcp);
        close(fd_udp);
        close(fd_tcp);
        fprintf(stderr, "Setsockopt failed\n");
        return 1;
    }


    if(bind(fd_tcp, res_tcp->ai_addr, res_tcp->ai_addrlen) == -1) {
        freeaddrinfo(res_udp);
        freeaddrinfo(res_tcp);
        close(fd_udp);
        close(fd_tcp);
        fprintf(stderr, "Error binding socket");
        return 1;
    }

    if(listen(fd_tcp, 1024) == -1) {
        freeaddrinfo(res_udp);
        freeaddrinfo(res_tcp);
        close(fd_udp);
        close(fd_tcp);
        fprintf(stderr, "Error listening socket");
        return 1;
    }


    FD_ZERO(&inputs);
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
            fprintf(stderr, "Error in select\n");
            freeaddrinfo(res_udp);
            freeaddrinfo(res_tcp);
            close(fd_udp);
            close(fd_tcp);
            return 1;
        default:
            if(FD_ISSET(fd_udp, &test_fds)) {
                if(server_udp(fd_udp, verbose) == 1) {
                    freeaddrinfo(res_udp);
                    freeaddrinfo(res_tcp);
                    close(fd_udp);
                    close(fd_tcp);
                    return 1;
                }
            }

            if(FD_ISSET(fd_tcp, &test_fds)) {
                struct sockaddr_in addr;
                socklen_t addrlen = sizeof(addr);

                int new_fd = accept(fd_tcp, (struct sockaddr*) &addr, &addrlen);
                if(new_fd == -1) {
                    freeaddrinfo(res_udp);
                    freeaddrinfo(res_tcp);
                    close(fd_udp);
                    close(fd_tcp);
                    return 1;
                }

                char host[NI_MAXHOST], port[NI_MAXSERV];
                if(verbose) {
                    if(getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof host, port, sizeof port, 0) == 0) {
                        printf("Request sent by [%s:%s]\n", host, port);
                    }
                    else
                        fprintf(stderr, "Error while getting hostname\n");
                }

                if(server_tcp(new_fd, verbose) == 1) {
                    freeaddrinfo(res_udp);
                    freeaddrinfo(res_tcp);
                    close(fd_udp);
                    close(fd_tcp);
                    close(new_fd);
                    return 1;
                }

                close(new_fd);
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

