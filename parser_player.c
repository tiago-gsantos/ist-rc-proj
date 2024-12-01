#include "parser_player.h"
#include <stdio.h>
#include <string.h>


int parse_start(char *buffer, char *request){
    unsigned int id_player;
    unsigned int time;
    char extra[2];

    if(sscanf(buffer, "%*s %u %u %1s", &id_player, &time, extra) != 2) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }
    

    if(id_player > 999999 || id_player < 100000 || time > 600) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }

    sprintf(request, "SNG %u %u\n", id_player, time);

    return 0;
}


int parse_try(char *buffer, char *request) {
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

    sprintf(request, "TRY %c %c %c %c\n", c[0], c[1], c[2], c[3]);

    return 0;
}


int parse_st(char *buffer, char *request) {
    char extra[2];

    int ret = sscanf(buffer, "%*s %1s", extra);
    if(ret != 0 && ret != -1) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }

    sprintf(request, "STR \n");

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


int parse_quit_exit(char *buffer, char *request) {
    char extra[2];

    int ret = sscanf(buffer, "%*s %1s", extra);
    if(ret != 0 && ret != -1) {
        fprintf(stderr, "Invalid arguments!\n");
        return 1;
    }

    sprintf(request, "QUT\n");

    return 0;
}


int parse_debug(char *buffer, char *request) {
    // TODO
}



