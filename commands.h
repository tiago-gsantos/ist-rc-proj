#ifndef COMMANDS
#define COMMANDS

#include <netdb.h>

int send_udp_request(char *request, int fd_udp, struct addrinfo *res, char *response);

void cmd_start(char *request, unsigned int *player_id, int *trial_num, int fd_udp, struct addrinfo *res);
void cmd_try(char *request, int *trial_num, int fd_udp, struct addrinfo *res);
void cmd_st(char *request, struct addrinfo *res);
void cmd_sb(char *request, struct addrinfo *res);
void cmd_quit(char *request, int *trial_num, int fd_udp, struct addrinfo *res);
void cmd_debug(char *request, unsigned int *player_id, int *trial_num, int fd_udp, struct addrinfo *res);

#endif // COMMANDS