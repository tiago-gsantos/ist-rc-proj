#include "parser_server.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


int validate_string_spaces(const char *str, int limit) {
    int space_flag = 0;
    int count = 0;

    size_t len = strlen(str);
    if (len == 0 || str[len - 1] != '\n')
        return 0;

    for (int i = 0; i < len - 1; i++) {
        if (isspace(str[i])) {
            if (space_flag)
                return 1;
            count ++;
            space_flag = 1;
        } else {
            space_flag = 0;
        }

        if(limit != 0 && count == limit)
            break;
    }

    if (isspace(str[0]) || (isspace(str[len - 2]) && limit == 0))
        return 1;

    return 0;
}



int parse_start(char *buffer, unsigned int *id, unsigned int *time){
    char extra[2];

    if(sscanf(buffer, "%*s %u %u %1s", id, time, extra) != 2 ||
        validate_string_spaces(buffer, 0) == 1) {
        return 1;
    }


    if(*id > 999999 || *id < 100000 || *time > 600) 
        return 1;
    
    return 0;
}


int parse_try(char *buffer, int *trial_num, char c[4], unsigned int *id) {
    char extra[2];

    if(sscanf(buffer, "%*s %u %c %c %c %c %d %1s", id, &c[0], &c[1], &c[2], &c[3], trial_num, extra) != 6 ||
        validate_string_spaces(buffer, 0) == 1)
        return 1;
    
    for(int i=0; i<4; i++) {
        if(strchr("RGBYOP", c[i]) == 0) 
            return 1;
    }

    if(*id > 999999 || *id < 100000) 
        return 1;

    return 0;
}


int parse_st(char *buffer, unsigned int *id) {
    char extra[2];

    if(sscanf(buffer, "%*s %u %1s", id, extra) != 1  ||
        validate_string_spaces(buffer, 0) == 1)
        return 1;

    if(*id > 999999 || *id < 100000) 
        return 1;

    return 0;
}


int parse_sb(char *buffer) {
    char extra[2];

    int ret = sscanf(buffer, "%*s %1s", extra);
    if(ret != 0 && ret != -1)
        return 1;

    if(validate_string_spaces(buffer, 0) == 1)
        return 1;

    return 0;
}


int parse_quit_exit(char *buffer, unsigned int *id) {
    char extra[2];

    if(sscanf(buffer, "%*s %u %1s", id, extra) != 1  ||
        validate_string_spaces(buffer, 0) == 1)
        return 1;

    if(*id > 999999 || *id < 100000) 
        return 1;

    return 0;
}


int parse_debug(char *buffer, unsigned int *id, unsigned int *time, char c[4]) {
    char extra[2];

    if(sscanf(buffer, "%*s %u %u %c %c %c %c %1s", id, time, &c[0], &c[1], &c[2], &c[3], extra) != 6  ||
        validate_string_spaces(buffer, 0) == 1) 
        return 1;
    
    if(*id > 999999 || *id < 100000 || *time > 600) 
        return 1;
    
    for(int i=0; i<4; i++) 
        if(strchr("RGBYOP", c[i]) == 0) 
            return 1;

    return 0;
}



