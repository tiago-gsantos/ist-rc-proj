#include "parser_player.h"
#include <stdio.h>
#include <string.h>


int parse_start(char *buffer, char *request, int trial_num, unsigned int *player_id){
    unsigned int id;
    unsigned int time;
    char extra[2];

    if(sscanf(buffer, "%*s %u %u %1s", &id, &time, extra) != 2) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }
    

    if(id > 999999 || id < 100000 || time > 600) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }

    *player_id = id;

    if(trial_num >= 1) {
        fprintf(stderr, "Invalid command! Quit the current game first.\n");
        return 1;
    }

    sprintf(request, "SNG %u %u\n", id, time);

    return 0;
}


int parse_try(char *buffer, char *request, unsigned int player_id, int trial_num) {
    char c[4];
    char extra[2];

    if(sscanf(buffer, "%*s %c %c %c %c %1s", &c[0], &c[1], &c[2], &c[3], extra) != 4) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }

    for(int i=0; i<4; i++) {
        if(strchr("RGBYOP", c[i]) == 0) {
            fprintf(stderr, "Invalid arguments!\n");
            return 1;
        }
    }

    if(player_id == 0 || trial_num == -1 || trial_num > 8) {
        fprintf(stderr, "Invalid command! Start a new game first.\n");
        return 1;
    }

    sprintf(request, "TRY %u %c %c %c %c %d\n", player_id, c[0], c[1], c[2], c[3], trial_num);

    return 0;
}


int parse_st(char *buffer, char *request, unsigned int player_id) {
    char extra[2];

    int ret = sscanf(buffer, "%*s %1s", extra);
    if(ret != 0 && ret != -1) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }

    if(player_id == 0) {
        fprintf(stderr, "Invalid command! Start a new game first.\n");
        return 1;
    }

    sprintf(request, "STR %u\n", player_id);

    return 0;
}


int parse_sb(char *buffer, char *request) {
    char extra[2];

    int ret = sscanf(buffer, "%*s %1s", extra);
    if(ret != 0 && ret != -1) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }

    sprintf(request, "SSB\n");

    return 0;
}


int parse_quit_exit(char *buffer, char *request, unsigned int player_id, int trial_num) {
    char extra[2];

    int ret = sscanf(buffer, "%*s %1s", extra);
    if(ret != 0 && ret != -1) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }

    if(player_id == 0 || trial_num == -1 || trial_num >= 8 ) {
        fprintf(stderr, "Invalid command! You are not playing any game.\n");
        return 1;
    }

    sprintf(request, "QUT %u\n", player_id);

    return 0;
}


int parse_debug(char *buffer, char *request, int trial_num, unsigned int *player_id) {
    unsigned int id;
    unsigned int time;
    char c[4];
    char extra[2];

    if(sscanf(buffer, "%*s %u %u %c %c %c %c %1s", &id, &time, &c[0], &c[1], &c[2], &c[3], extra) != 6) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }
    

    if(id > 999999 || id < 100000 || time > 600) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }

    *player_id = id;

    for(int i=0; i<4; i++) {
        if(strchr("RGBYOP", c[i]) == 0) {
            fprintf(stderr, "Invalid arguments!\n");
            return 1;
        }
    }

    if(trial_num >= 1) {
        fprintf(stderr, "Invalid command! Quit the current game first.\n");
        return 1;
    }

    sprintf(request, "DBG %u %u %c %c %c %c\n", id, time, c[0], c[1], c[2], c[3]);

    return 0;
}



