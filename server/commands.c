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


void number_blacks_and_whites(char code[5], char try[5], int num_b_w[2]){
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

    if(secs != NULL)
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


ssize_t format_data(FILE *file, char status[8], unsigned int player_id, char *formatted_data){
    char mode;
    char code[5];
    unsigned int time_limit;
    char start_time[9];
    char start_date[11];
    char end_time[9];
    char end_date[11];
    char line[128];
    TRYSLIST trys;
    unsigned int current_seconds;
    unsigned int start_seconds;
    int number_trials = -1;

    fseek(file, 0, SEEK_SET);

    while(fgets(line, sizeof line, file) != NULL) { 
        if(number_trials == -1){
            if(sscanf(line, "%*s %c %s %u %s %s %u", 
            &mode, code, &time_limit, start_date, start_time, &start_seconds) != 6){
                return 0;
            }
            if(!status){
                sprintf(line, "Active game found for player %u\n", player_id);
                strcpy(formatted_data, line);
                sprintf(line, "Game initiated: %s %s with %u seconds to be completed\n",
                start_date, start_time, time_limit);
                strcat(formatted_data, line);
            }
            else{
                sprintf(line, "Last finalized game for player %u\n", player_id);
                strcpy(formatted_data, line);
                sprintf(line, "Game initiated: %s %s with %u seconds to be completed\n",
                start_date, start_time, time_limit);
                strcat(formatted_data, line);
                if(mode == 'D')
                    sprintf(line, "Mode: DEBUG Secret code: %s\n", code);
                else
                    sprintf(line, "Mode: PLAY Secret code: %s\n", code);
                strcat(formatted_data, line);
            }
            number_trials++;
        }
        else{
            if(line[0] == 'T'){
                if(sscanf(line, "%*s %s %d %d %u", trys.codes[number_trials], &trys.num_blacks[number_trials],
                &trys.num_whites[number_trials], &trys.time[number_trials]) != 4){
                    return 0;
                }
                number_trials++;
            }
            else{
                if(sscanf(line, "%s %s %u", end_date, end_time, &current_seconds) != 3){
                    return 0;
                }
            }
        }
    } 

    sprintf(line, "\n\t--- Transactions found: %d ---\n\n", number_trials);
    strcat(formatted_data, line);

    for(int i = 0; i < number_trials; i++){
        sprintf(line, "Trial: %s, nB: %d, nW: %d at %us\n", trys.codes[i],
        trys.num_blacks[i], trys.num_whites[i], trys.time[i]);
        strcat(formatted_data, line);
    }

    if(!status){
        get_current_time(&current_seconds, NULL);
        sprintf(line, "-- %u seconds remaining to be completed --\n", time_limit - (current_seconds - start_seconds));
    }
    else
        sprintf(line, "Termination: %s at %s %s, Duration: %us\n", status, end_date,
        end_time, current_seconds);
    strcat(formatted_data, line);

    return strlen(formatted_data);
}

void get_game_status(char* file_path, char *status){
    char status_char;
    status_char = file_path[29];

    switch (status_char){
        case 'W':
            strcpy(status, "WIN");
            break;
        case 'Q':
            strcpy(status, "QUIT");
            break;
        case 'F':
            strcpy(status, "FAIL");
            break;
        case 'T':
            strcpy(status, "TIMEOUT");
            break;
        default:
            strcpy(status, "UNKNOWN");
    }

    return;
}



void save_game(FILE *file, unsigned int player_id, char status, unsigned int num_trials) {
    char date[11];
    char time[9];
    unsigned int start_secs;
    char mode;
    char code[5];
    unsigned int time_limit;
    
    fseek(file, 0, SEEK_SET);

    char line[64];
    if(fgets(line, sizeof line, file) != NULL) { 
        if(sscanf(line, "%*s %c %s %u %4s-%2s-%s %2s:%2s:%s %u", &mode, code, &time_limit, date, date+4, date+6, time, time+2, time+4, &start_secs) != 10)
            return;
    } else
        return;
    

    char formatted_time[20];
    unsigned int current_secs;
    get_current_time(&current_secs, formatted_time);

    if(current_secs - start_secs > time_limit){
        sprintf(line, "%s %u\n", formatted_time, time_limit);
    }
    else
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
    file = fopen(file_path, "r+");
    if(file){
        char buffer[1024];
        fgets(buffer, sizeof buffer, file);
        if(fgets(buffer, sizeof buffer, file) != NULL) {
            
            int game_status = get_game_info(file, NULL, &secs);
            if(game_status == -1) {
                strcpy(response, "RSG ERR\n");
                return;
            }
            else if(game_status == 0) {
                sprintf(response, "RSG NOK\n");
                return;
            }
            else if(game_status == 1)
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


void cmd_try(char *response, unsigned int player_id, int trial_num, char try[5]){
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


int find_top_scores(SCORELIST *list) {
    struct dirent **file_list;
    int num_entries, ifile;
    char file_name[300];
    FILE *fp;
    
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
                    fscanf(fp, "%s %u %s %u %s",
                           list->score[ifile],
                           &list->PLID[ifile],
                           list->code[ifile],
                           &list->num_tries[ifile],
                           list->mode[ifile]);

                    fclose(fp);
                    ++ifile;
                }
            }
            free(file_list[num_entries]);
        }
        free(file_list);
    }

    list->num_scores = ifile;
    return ifile;
}


void cmd_st(char *response, unsigned int player_id){
    char formatted_data[1024];
    char status[8];
    ssize_t form_data_size;
    
    char file_path[35];
    sprintf(file_path, "./GAMES/GAMES_%u.txt", player_id);

    FILE *file;
    file = fopen(file_path, "r+");
    if(file){
        unsigned int secs;
        int game_status = get_game_info(file, NULL, &secs);
        if(game_status == -1) {
            strcpy(response, "RST ERR\n");
            fclose(file);
            return;
        }
        else if(game_status == 0) {
            form_data_size = format_data(file, NULL, player_id, formatted_data);
            
            if(form_data_size == 0)
                strcpy(response, "RST ERR\n");
            else
                sprintf(response, "RST ACT STATUS_%u.txt %ld %s\n", player_id, form_data_size, formatted_data);

            fclose(file);
            return;
        }
        else if(game_status == 1) {
            save_game(file, player_id, 'T', 0);
            fclose(file);
        }
    }
    
    if(find_last_game(player_id, file_path) == 1){
        file = fopen(file_path, "r");

        get_game_status(file_path, status);
        
        form_data_size = format_data(file, status, player_id, formatted_data);

        if(form_data_size == 0)
            strcpy(response, "RST ERR\n");
        else
            sprintf(response, "RST FIN STATUS_%u.txt %ld %s\n", player_id, form_data_size, formatted_data);

        fclose(file);
    }
    else
        strcpy(response, "RST NOK\n");

    return;
}


void cmd_sb(char *response){
    char file_data[1024];
    char line[64];
    char date[20];
    SCORELIST score_list;

    memset(&score_list, 0, sizeof(SCORELIST));
    
    if(find_top_scores(&score_list) == 0) {
        strcpy(response, "RSS EMPTY\n");
        return;
    }

    strcpy(file_data, "---------------------- TOP 10 SCORES ----------------------\n\n");
    strcat(file_data, "           SCORE   PLAYER   CODE   TRIALS   MODE\n");


    for(unsigned int i = 0; i < score_list.num_scores; i++) {
        if(i+1 != 10)
            sprintf(line, "     %u -    %s    %u   %s      %u     %s\n", i+1, score_list.score[i], score_list.PLID[i], score_list.code[i], score_list.num_tries[i], score_list.mode[i]);
        else
            sprintf(line, "    %u -    %s    %u   %s      %u     %s\n", i+1, score_list.score[i], score_list.PLID[i], score_list.code[i], score_list.num_tries[i], score_list.mode[i]);

        strcat(file_data, line);
    }

    if(score_list.num_scores < 10)
        strcat(file_data, "     (no more games)\n");

    get_current_time(NULL, date);
    // YYYY-MM-DD HH:MM:SS
    date[10] = '_';
    date[13] = '-';
    date[16] = '-';
    
    sprintf(response, "RSS OK %s.txt %ld %s\n", date, strlen(file_data), file_data);
    
    return;
}


void cmd_quit(char *response, unsigned int player_id){
    char file_path[32];
    char code[5];
    sprintf(file_path, "./GAMES/GAMES_%u.txt", player_id);

    FILE *file;
    file = fopen(file_path, "r+");
    if(file){
        unsigned int secs;
        int game_status = get_game_info(file, NULL, &secs);
        if(game_status == -1) {
            strcpy(response, "RQT ERR\n");
            return;
        }
        else if(game_status == 0) {
            char line[64];

            fseek(file, 0, SEEK_SET);
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
        else if(game_status == 1) {
            save_game(file, player_id, 'T', 0);

            strcpy(response, "RQT NOK\n");
        }
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
    file = fopen(file_path, "r+");
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