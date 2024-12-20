#ifndef COMMANDS
#define COMMANDS

#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>

int send_udp_request(char *request, int fd_udp, struct addrinfo *res, char *response);
int send_tcp_request(char *request, struct addrinfo *res, char *response);

int validate_string_spaces(const char *str, int limit);
int create_file(char *f_name, char *f_data);

int cmd_start(char *request, unsigned int *player_id, int *trial_num, int fd_udp, struct addrinfo *res);
int cmd_try(char *request, unsigned int player_id, int *trial_num, int fd_udp, struct addrinfo *res);
int cmd_st(char *request, struct addrinfo *res);
int cmd_sb(char *request, struct addrinfo *res);
int cmd_quit(char *request, int *trial_num, int fd_udp, struct addrinfo *res);
int cmd_debug(char *request, unsigned int *player_id, int *trial_num, int fd_udp, struct addrinfo *res);

#endif // COMMANDS