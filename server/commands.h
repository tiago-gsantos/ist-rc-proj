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


typedef struct {
    char score[10][4];
    unsigned int PLID[10];
    char code[10][5];
    unsigned int num_tries[10];
    char mode[10][6];
    unsigned int num_scores;
} SCORELIST;

typedef struct {
    char codes[8][5];
    int num_whites[8];
    int num_blacks[8];
    unsigned int time[8];
} TRYSLIST;


int cmd_start(char *response, unsigned int player_id, unsigned int time);
int cmd_try(char *response, unsigned int player_id, int trial_num, char try[5]);
void cmd_st(char *response, unsigned int player_id);
void cmd_sb(char *response);
int cmd_debug(char *response, unsigned int player_id, unsigned int game_time, char c[4]);
int cmd_quit(char *response, unsigned int player_id);
int create_file(char *f_name, char *f_data);
void generate_color_code(char c[4]);
void number_blacks_and_whites(char code[5], char try[5], int num_b_w[2]);
void get_current_time(unsigned int *secs, char *formatted_time);
int get_game_info(FILE *file, char code[5], unsigned int *current_seconds);
void get_score(unsigned int time, unsigned int number_trials, char mode, char score[4]);
int save_game(FILE *file, unsigned int player_id, char status, unsigned int num_trials);
int find_last_game(unsigned int player_id, char *file_name);
ssize_t format_data(FILE *file, char status[8], unsigned int player_id, char *formatted_data);
void get_game_status(char* file_path, char *status);

#endif // COMMANDS