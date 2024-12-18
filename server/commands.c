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


#define WAIT_TIME_LIMIT 15 // ???
#define TRY_LIMIT 3
#define RESPONSE_LEN 32


int create_file(char *f_name, char *f_data) {
    FILE *file = fopen(f_name, "w");
    if(file == NULL)
        return 1;

    fprintf(file, "%s", f_data);
    fflush(file);
    fclose(file);
    return 0;
}


void generate_color_code(char c[4]){
    char possible_colors[] = {'R', 'G', 'B', 'Y', 'O', 'P'};
   
    srand(time(NULL));
    for (int i = 0; i < 4; i++) {
        int random_idx = rand() % 6; 
        c[i] = possible_colors[random_idx];
    }
}


void number_blacks_and_whites(char code[5], char try[4], int num_b_w[2]){
    int num_whites = 0;
    int num_blacks = 0;
    int i, j;
    for(i = 0; i < 4; i++){
        if(try[i] == code[i]){
            num_blacks++;
            try[i] = 'N';
            code[i] = 'M';
        }
    }
    for(i = 0; i < 4; i++){
        for(j = 0; j < 4; j++){
            if(try[i] == code[j]){
                num_whites++;
                try[i] = 'N';
                code[j] = 'M';
                break;
            }
        }
    }

    num_b_w[0] = num_blacks;
    num_b_w[1] = num_whites;
}

void get_current_time(unsigned int *secs, char *formatted_time) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    if(formatted_time != NULL)
        strftime(formatted_time, 20, "%Y-%m-%d %H:%M:%S", local);

    *secs = (int)now;

    return;
}


int get_game_info(FILE *file, char code[5], unsigned int *time_passed) {
    char line[64];
    unsigned int start_seconds;
    unsigned int time_limit;
    unsigned int current_seconds;

    if(fgets(line, sizeof line, file) != NULL) { 
        if(code) {
            if(sscanf(line, "%*s %*c %s %u %*s %*s %u", code, &time_limit, &start_seconds) != 3)
                return -1;
        } else {
            if(sscanf(line, "%*s %*c %*s %u %*s %*s %u", &time_limit, &start_seconds) != 2)
                return -1;
        }
    } 
    else
        return -1;

    get_current_time(&current_seconds, NULL);

    *time_passed = current_seconds - start_seconds;

    if(*time_passed >= time_limit)
        return 1;

    return 0;
}


void save_game(FILE *file, unsigned int player_id, char status, unsigned int num_trials) {
    char date[11];
    char time[9];
    unsigned int start_secs;
    char mode;
    char code[5];
    
    fseek(file, 0, SEEK_SET);

    char line[64];
    if(fgets(line, sizeof line, file) != NULL) { 
        if(sscanf(line, "%*s %c %s %*u %4s-%2s-%s %2s:%2s:%s %u", &mode, code, date, date+4, date+6, time, time+2, time+4, &start_secs) != 9)
            return;
    } else
        return;
    

    char formatted_time[20];
    unsigned int current_secs;
    get_current_time(&current_secs, formatted_time);


    sprintf(line, "%s %u\n", formatted_time, current_secs - start_secs);

    fseek(file, 0, SEEK_END);
    fprintf(file, "%s", line);
    fflush(file);

    
    char dir_path[15];
    sprintf(dir_path, "./GAMES/%u", player_id);
    
    mkdir(dir_path, 0755); // erro ?
    
    char file_path[64];
    sprintf(file_path, "./GAMES/GAMES_%u.txt", player_id);

    char new_file_path[64];
    sprintf(new_file_path, "%s/%s_%s_%c.txt", dir_path, date, time, status);

    rename(file_path, new_file_path);

    if(status == 'W') {
        char day[3], month[3], year[5];
        if(sscanf(formatted_time, "%4s-%2s-%s %2s:%2s:%s", year, month, day, time, time+2, time+4) != 6)
            return;

        sprintf(date, "%s%s%s", day, month, year);

        char score[4];
        get_score(current_secs - start_secs, num_trials, mode, score);

        sprintf(file_path, "./SCORES/%s_%u_%s_%s.txt", score, player_id, date, time);

        if(mode == 'D')
            sprintf(line, "%s %u %s %u %s", score, player_id, code, num_trials, "DEBUG");
        else if(mode == 'P')
            sprintf(line, "%s %u %s %u %s", score, player_id, code, num_trials, "PLAY");

        create_file(file_path, line); // erro ?
    }
    
    
    return;
}


void get_score(unsigned int time, unsigned int number_trials, char mode, char score[4]){
    int time_score;
    int num_trials_score;

    if(time >= 250) time_score = 0;
    else time_score = 50 - 0.20 * time;

    num_trials_score = 50 - (number_trials - 1) * 7;

    if(mode == 'D') {
        sprintf(score, "%03d", (time_score + num_trials_score)/10);
    }
    else{
        sprintf(score, "%03d", time_score + num_trials_score);
    }
    
    return ;
}



void cmd_start(char *response, unsigned int player_id, unsigned int game_time) {
    char file_data[64];
    char c[4];
    char formatted_time[20];
    unsigned int secs;
    
    char file_path[32];
    sprintf(file_path, "./GAMES/GAMES_%u.txt", player_id);

    FILE *file;
    file = fopen(file_path, "r");
    if(file){
        int game_status = get_game_info(file, NULL, &secs);
        if(game_status == -1) {
            strcpy(response, "RSG ERR\n");
            return;
        }
        else if(game_status == 0) {
            sprintf(response, "RSG NOK\n");
            return;
        }
        else if(game_status == 1) {
            save_game(file, player_id, 'T', 0);
        }
    }

    generate_color_code(c);

    get_current_time(&secs, formatted_time);
    
    sprintf(file_data, "%u P %c%c%c%c %u %s %u\n", player_id, c[0], c[1], c[2], c[3], game_time, formatted_time, secs);

    if(create_file(file_path, file_data) != 0){
        strcpy(response, "RSG ERR\n");
        return;
    }

    strcpy(response, "RSG OK\n");

    return;
}


void cmd_try(char *response, unsigned int player_id, int trial_num, char try[4]){
    char file_path[32];
    char code[5];
    char past_trial[5];
    int is_duplicated = 0;
    int is_last_duplicated = 0;
    int num_b_w[2];
    unsigned int trial_seconds;
    int server_trial_num = 0;
    char line[64];
    char status = 0;
    
    sprintf(file_path, "./GAMES/GAMES_%u.txt", player_id);

    FILE *file;
    file = fopen(file_path, "r+"); // fechar ficheiro nos returns
    if(!file || trial_num < 0 || trial_num > 8) {
        strcpy(response, "RTR NOK\n");
        return;
    }


    int game_status = get_game_info(file, code, &trial_seconds);
    if(game_status == -1) {
        strcpy(response, "RTR ERR\n");
        return;
    }
    else if(game_status == 1) {
        sprintf(response, "RTR ETM %c %c %c %c\n", code[0], code[1], code[2], code[3]);
        save_game(file, player_id, 'T', 0);
        return;
    }

    while(fgets(line, sizeof line, file) != NULL){
        sscanf(line, "%*s %s", past_trial);
        server_trial_num++;
        if(strncmp(try, past_trial, 4) == 0){
            if(server_trial_num != trial_num){
                strcpy(response, "RTR DUP\n");
                is_duplicated = 1;
            }
            else is_last_duplicated = 1;
        }
    }

    if(trial_num != server_trial_num + 1){
        strcpy(response, "RTR INV\n");
        save_game(file, player_id, 'F', 0);
        return;
    }

    if(is_duplicated) return;

    char code_copy[5], try_copy[5];
    strcpy(code_copy, code);
    strcpy(try_copy, try);
    number_blacks_and_whites(code_copy, try_copy, num_b_w);

    if(trial_num != 8 || num_b_w[0] == 4){
        sprintf(response, "RTR OK %d %d %d\n", trial_num, num_b_w[0], num_b_w[1]);
        if(num_b_w[0] == 4)
            status = 'W';
    }
    else{
        sprintf(response, "RTR ENT %c %c %c %c\n", code[0], code[1], code[2], code[3]);
        status = 'F';
    }

    if(is_last_duplicated == 0){
        sprintf(line, "T: %c%c%c%c %d %d %d\n",
        try[0], try[1], try[2], try[3], num_b_w[0], num_b_w[1], trial_seconds);
        
        fseek(file, 0, SEEK_END);

        fprintf(file, "%s", line);
        fflush(file);
    }

    if(status)
        save_game(file, player_id, status, trial_num);
    
    return;
}


int find_last_game(unsigned int player_id, char *file_name) {
    struct dirent **file_list;
    int num_entries, found;
    char dir_name[20];
    sprintf(dir_name, "GAMES/%u/", player_id);
    num_entries = scandir(dir_name, &file_list, 0, alphasort);
    found = 0;
    if(num_entries <= 0)
        return 0;
    else {
        while (num_entries--){
        if(file_list[num_entries]->d_name[0] != '.' && !found) {
            sprintf(file_name, "GAMES/%u/%s", player_id, file_list[num_entries]->d_name);
            found =1;
        }
        free(file_list[num_entries]);
        }
        free(file_list);
    }
    return found;
}

/*
int find_top_scores(SCORELIST *list) {
    struct dirent **file_list;
    int num_entries, ifile;
    char file_name[300];
    FILE *fp;
    char mode[8];
    
    num_entries = scandir("SCORES/", &file_list, 0, alphasort);
    if (num_entries <= 0)
        return 0;
    else {
        ifile = 0;
        while (num_entries--) {
            if (file_list[num_entries]->d_name[0] != '.' && ifile < 10) {
                sprintf(file_name, "SCORES/%s", file_list[num_entries]->d_name);
                fp = fopen(file_name, "r");
                if (fp != NULL) {
                    fscanf(fp, "%d %s %s %d %s",
                           &list->score[ifile],
                           list->PLID[ifile],
                           list->colcode[ifile],
                           &list->notries[ifile],
                           mode);
                    
                    if (!strcmp(mode, "PLAY"))
                        list->mode[ifile] = MODEPLAY;
                    if (!strcmp(mode, "DEBUG"))
                        list->mode[ifile] = MODEDEBUG;

                    fclose(fp);
                    ++ifile;
                }
            }
            free(file_list[num_entries]);
        }
        free(file_list);
    }

    list->nscores = ifile;
    return ifile;
}
*/


void cmd_st(char *response, unsigned int player_id){
    char code[5];
    char file_data[1024] = {0};
    size_t file_size;
    
    char file_path[35];
    sprintf(file_path, "./GAMES/GAMES_%u.txt", player_id);

    FILE *file;
    file = fopen(file_path, "r");
    if(file){
        file_size = fread(file_data, 1, sizeof(file_data) - 1, file);
        sprintf(response, "RST ACT GAMES_%u.txt %ld %s\n", player_id, file_size, format_data(file_data, 0));

        fclose(file);
    }
    else if(find_last_game(player_id, file_path) == 1){
        file = fopen(file_path, "r");

        file_size = fread(file_data, 1, sizeof(file_data) - 1, file);
        char status = get_game_status(file_path);
        sprintf(response, "RST FIN GAMES_%u.txt %ld %s\n", player_id, file_size, format_data(file_data, status));

        fclose(file);
    }
    else
        strcpy(response, "RST NOK\n");

    return;
}


void cmd_sb(char *response){
    char code[5];
    

    return;
}


void cmd_quit(char *response, unsigned int player_id){
    char file_path[32];
    char code[5];
    sprintf(file_path, "./GAMES/GAMES_%u.txt", player_id);

    FILE *file;
    file = fopen(file_path, "r+");
    if(file){
        char line[64];

        if(fgets(line, sizeof line, file) != NULL) { 
            if(sscanf(line, "%*s %*c %s", code) != 1){
                strcpy(response, "RQT ERR\n");
                return;
            }
        } else {
            strcpy(response, "RQT ERR\n");
            return;
        }
        
        save_game(file, player_id, 'Q', 0);
        
        sprintf(response, "RQT OK %c %c %c %c\n", code[0], code[1], code[2], code[3]);
    }
    else
        strcpy(response, "RQT NOK\n");

    return;
}

void cmd_debug(char *response, unsigned int player_id, unsigned int game_time, char c[4]) {
    char file_data[64];
    char formatted_time[20];
    unsigned int secs;
    
    char file_path[32];
    sprintf(file_path, "./GAMES/GAMES_%u.txt", player_id);

    FILE *file;
    file = fopen(file_path, "r");
    if(file){
        int game_status = get_game_info(file, NULL, &secs);
        if(game_status == -1) {
            strcpy(response, "RDB ERR\n");
            return;
        }
        else if(game_status == 0) {
            sprintf(response, "RDB NOK\n");
            return;
        }
        else if(game_status == 1) {
            //quit
        }
    }

    get_current_time(&secs, formatted_time);
    
    sprintf(file_data, "%u D %c%c%c%c %u %s %u\n", player_id, c[0], c[1], c[2], c[3], game_time, formatted_time, secs);

    if(create_file(file_path, file_data) != 0){
        strcpy(response, "RDB ERR\n");
        return;
    }

    strcpy(response, "RDB OK\n");

    return;
}