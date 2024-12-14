#ifndef COMMANDS
#define COMMANDS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>

void cmd_start(char *response, unsigned int player_id, unsigned int time);
void cmd_try(char *response, unsigned int player_id, int trial_num, char c[4]);
int create_file(char *f_name, char *f_data);
void generate_color_code(char c[4]);
void number_whites_and_blacks(char code[4], char try[4], int num_whites_and_blacks[2]);

#endif // COMMANDS