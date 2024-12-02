#ifndef COMMANDS
#define COMMANDS

int cmd_start(char *request, unsigned int *player_id, int *trial_num, int fd_udp, struct addrinfo *res);

#endif // COMMANDS