#ifndef COMMANDS
#define COMMANDS

#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>


void cmd_start(char *response, unsigned int player_id, unsigned int time);
void cmd_try(char *response, unsigned int player_id, int trial_num, char try[4]);
void cmd_st(char *response, unsigned int player_id);
void cmd_sb(char *response);
void cmd_debug(char *response, unsigned int player_id, unsigned int game_time, char c[4]);
void cmd_quit(char *response, unsigned int player_id);
int create_file(char *f_name, char *f_data);
void generate_color_code(char c[4]);
void number_blacks_and_whites(char code[5], char try[4], int num_b_w[2]);
void get_current_time(unsigned int *secs, char *formatted_time);
int get_game_info(FILE *file, char code[5], unsigned int *current_seconds);
void get_score(unsigned int time, unsigned int number_trials, char mode, char score[4]);
void save_game(FILE *file, unsigned int player_id, char status, unsigned int num_trials);
int find_last_game(unsigned int player_id, char *file_name);
char *format_data(char *file_data, char status);
char get_game_status(char* file_path);

#endif // COMMANDS