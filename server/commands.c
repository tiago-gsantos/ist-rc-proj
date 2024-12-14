#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

#define WAIT_TIME_LIMIT 15
#define TRY_LIMIT 3
#define RESPONSE_LEN 32


int create_file(char *f_name, char *f_data) {
    FILE *file = fopen(f_name, "w");
    if(file == NULL)
        return 1;

    fprintf(file, "%s", f_data);
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


void number_whites_and_blacks(char code[4], char try[4], int num_w_b[2]){
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

    num_w_b[0] = num_whites;
    num_w_b[1] = num_blacks;
}


void cmd_start(char *response, unsigned int player_id, unsigned int game_time) {
    char file_data[64];
    char c[4];
    
    char file_path[32];
    sprintf(file_path, "./GAMES/GAMES_%u.txt", player_id);

    FILE *file;
    file = fopen(file_path, "r");
    if(file)
        strcpy(response, "RSG NOK\n");
    else {
        generate_color_code(c);
        
        time_t now = time(NULL);
        struct tm *local = localtime(&now);

        char formatted_time[20];
        strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", local);

        int s = (local->tm_hour * 3600) + (local->tm_min * 60) + local->tm_sec;
        
        sprintf(file_data, "%u P %c%c%c%c %u %s %d\n", player_id, c[0], c[1], c[2], c[3], game_time, formatted_time, s);

        if(create_file(file_path, file_data) != 0){
            strcpy(response, "RSG ERR\n");
            return;
        }

        strcpy(response, "RSG OK\n");
    }

    return;
}


void cmd_try(char *response, unsigned int player_id, int trial_num, char try[4]){
    char file_path[32];
    char code[4];
    int num_w_b[2];
    sprintf(file_path, "./GAMES/GAMES_%u.txt", player_id);

    FILE *file;
    file = fopen(file_path, "r+");
    if(!file || trial_num < 0 || trial_num > 8) {
        strcpy(response, "RTR NOK\n");
        return;
    }

    char line[64];
    if(fgets(line, sizeof line, file) != NULL) {
        for(int i = 0; i < 4; i++)
            code[i] = line[9+i];
    }

    number_whites_and_blacks(code, try, num_w_b);
    
    sprintf(response, "RTR OK %d %d %d\n", trial_num, num_w_b[0], num_w_b[1]);

    return;
}

/*int cmd_st(char *request, struct addrinfo *res){
    char res_cmd[4];
    char status[4];
    char f_name[256];
    unsigned int f_size;


    char *response = (char *) malloc(sizeof(char) * 512);
    if (response == NULL) {
        fprintf(stderr, "Error while communicating with server!\n");
        return 1;
    }


    if(send_tcp_request(request, res, response) == 1) {
        fprintf(stderr, "Error while communicating with server!\n");
        free(response);
        return 1;
    }


    if(sscanf(response, "%3s %4s", res_cmd, status) != 2){
        fprintf(stderr, "Invalid response from server!\n");
        free(response);
        return 0;
    }
   
    if(strcmp(res_cmd, "RST") != 0){
        fprintf(stderr, "Invalid response from server!\n");
        free(response);
        return 0;
    }

    if(strcmp(status, "ACT") == 0 || strcmp(status, "FIN") == 0){
        if(sscanf(response, "%*s %*s %s %u", f_name, &f_size) != 2){
            fprintf(stderr, "Invalid response from server!\n");
            free(response);
            return 0;
        }
        
        char *f_data = &response[strlen(response) - f_size];
        printf("%s %u\n%s", f_name, f_size, f_data);
        
        if(create_file(f_name, f_data) == 1) {
            fprintf(stderr, "Error creating file.\n");
            free(response);
            return 0;
        }
    }
    else if(strcmp(status, "NOK") == 0){
        printf("Invalid command! You are not playing any game.\n");
    }
    else if(strcmp(status, "ERR") == 0){
        fprintf(stderr, "Invalid response from server!\n");
    }
    else{
        fprintf(stderr, "Invalid response from server!\n");
    }

    free(response);
    return 0;
}


int cmd_sb(char *request, struct addrinfo *res){
    char res_cmd[4];
    char status[6];
    char f_name[256];
    unsigned int f_size;


    char *response = (char *) malloc(sizeof(char) * 512);
    if (response == NULL) {
        fprintf(stderr, "Error while communicating with server!\n");
        return 1;
    }


    if(send_tcp_request(request, res, response) == 1) {
        fprintf(stderr, "Error while communicating with server!\n");
        free(response);
        return 1;
    }


    if(sscanf(response, "%3s %6s", res_cmd, status) != 2){
        fprintf(stderr, "Invalid response from server!\n");
        free(response);
        return 0;
    }
   
    if(strcmp(res_cmd, "RSS") != 0){
        fprintf(stderr, "Invalid response from server!\n");
        free(response);
        return 0;
    }

    if(strcmp(status, "OK") == 0){
        if(sscanf(response, "%*s %*s %s %u", f_name, &f_size) != 2){
            fprintf(stderr, "Invalid response from server!\n");
            free(response);
            return 0;
        }
        
        char *f_data = &response[strlen(response) - f_size];
        printf("%s", f_data);
        
        if(create_file(f_name, f_data) == 1) {
            fprintf(stderr, "Error creating file.\n");
            free(response);
            return 0;
        }
    }
    else if(strcmp(status, "EMPTY") == 0){
        printf("No game was yet won by any player!\n");
    }
    else if(strcmp(status, "ERR") == 0){
        fprintf(stderr, "Invalid response from server!\n");
    }
    else{
        fprintf(stderr, "Invalid response from server!\n");
    }

    free(response);
    return 0;
}


int cmd_quit(char *request, int *trial_num, int fd_udp, struct addrinfo *res){
    char response[RESPONSE_LEN];
    char res_cmd[4];
    char status[4];
    char extra[1];
    char c[4];

    
    if(send_udp_request(request, fd_udp, res, response) == 1) {
        fprintf(stderr, "Error while communicating with server!\n");
        return 1;
    }

    if(sscanf(response, "%3s %4s", res_cmd, status) != 2){
        fprintf(stderr, "Invalid response from server!\n");
        return 0;
    }
   
    if(strcmp(res_cmd, "RQT") != 0){
        fprintf(stderr, "Invalid response from server!\n");
        return 0;
    }

    if(strcmp(status, "OK") == 0){
        if(sscanf(response, "%*s %*s %c %c %c %c %1s", &c[0], &c[1], &c[2], &c[3], extra) != 4) {
            fprintf(stderr, "Invalid response from server!\n");
            return 0; 
        }

        fprintf(stderr, "You quit the game! The color code was: %c %c %c %c\n", c[0], c[1], c[2], c[3]);
        *trial_num = -1;
    }
    else if(strcmp(status, "NOK") == 0){
        printf("You are not playing any game.\n");
        *trial_num = -1;
    }
    else if(strcmp(status, "ERR") == 0){
        printf("ERROR\n");
    }
    else{
        fprintf(stderr, "Invalid response from server!\n");
    }

    return 0;
}


int cmd_debug(char *request, unsigned int *player_id, int *trial_num, int fd_udp, struct addrinfo *res) {
    char response[RESPONSE_LEN];
    char res_cmd[4];
    char status[4];
    char extra[1];

    
    if(send_udp_request(request, fd_udp, res, response) == 1) {
        fprintf(stderr, "Error while communicating with server!\n");
        return 1;
    }

    if(sscanf(response, "%3s %3s %1s", res_cmd, status, extra) != 2){
        fprintf(stderr, "Invalid response from server!\n");
        return 0;
    }

    if(strcmp(res_cmd, "RDB") != 0){
        fprintf(stderr, "Invalid response from server!\n");
        return 0;
    }

    if(strcmp(status, "ERR") == 0){
        fprintf(stderr, "Invalid arguments!\n");
    }
    else if(strcmp(status, "NOK") == 0){
        fprintf(stderr, "Invalid command! Quit the current game first.\n");
    }
    else if(strcmp(status, "OK") == 0){
        sscanf(request, "%*s %u %*s", player_id);
        *trial_num = 1;
        printf("The game has started!\n");
    }
    else{
        fprintf(stderr, "Invalid response from server!\n");
    }

    return 0;
}*/


