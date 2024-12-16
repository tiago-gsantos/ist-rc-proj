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


void number_blacks_and_whites(char code[5], char try[4], int num_w_b[2]){
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

    num_w_b[0] = num_blacks;
    num_w_b[1] = num_whites;
}

void get_current_time(unsigned int *secs, char *formatted_time) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    if(formatted_time != NULL)
        strftime(formatted_time, 20, "%Y-%m-%d %H:%M:%S", local);

    *secs = (int)now;

    return;
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
        
        char formatted_time[20];
        unsigned int secs;

        get_current_time(&secs, formatted_time);
        
        sprintf(file_data, "%u P %c%c%c%c %u %s %u\n", player_id, c[0], c[1], c[2], c[3], game_time, formatted_time, secs);

        if(create_file(file_path, file_data) != 0){
            printf("create file\n");
            strcpy(response, "RSG ERR\n");
            return;
        }

        strcpy(response, "RSG OK\n");
    }

    return;
}


void cmd_try(char *response, unsigned int player_id, int trial_num, char try[4]){
    //buscar tempo - tempo limite, tempo em segundos, calcular o novo tempo (comparar valores) - primeira coisa a fazer
    //buscar número da última tentativa - segunda coisa a fazer
    //buscar todos os código que já metemos nesta tentativa - terceira coisa a fazer
    //Verificar tempo
    //Verificar se é jogada - 1 e o código da passada (responder com OK)
    //Verificar se é duplicada
    //Verificar se é a última jogada (acertou ou falhou) - mostrar o código
    
    char file_path[32];
    char code[5];
    char past_trial[5];
    int is_duplicated = 0;
    int is_last_duplicated = 0;
    int num_b_w[2];
    unsigned int time_limit;
    unsigned int start_seconds;
    unsigned int trial_seconds;
    int server_trial_num = 0;
    
    sprintf(file_path, "./GAMES/GAMES_%u.txt", player_id);

    FILE *file;
    file = fopen(file_path, "r+"); // fechar ficehiro nos returns
    if(!file || trial_num < 0 || trial_num > 8) {
        strcpy(response, "RTR NOK\n");
        return;
    }

    char line[64];
    if(fgets(line, sizeof line, file) != NULL) {
        if(sscanf(line, "%*s %*c %4s %u %*s %*s %u", code, &time_limit, &start_seconds) != 3){
            strcpy(response, "RTR ERR\n");
            return;
        }
    } else {
        strcpy(response, "RTR ERR\n");
        return;
    }

    get_current_time(&trial_seconds, NULL);

    if(trial_seconds >= time_limit + start_seconds){
        sprintf(response, "RTR ETM %c %c %c %c\n", code[0], code[1], code[2], code[3]);
        //Fazer quit do jogo
        return;
    }

    while(fgets(line, sizeof line, file) != NULL){
        sscanf(line, "%d: %s", &server_trial_num, past_trial);
        if(strncmp(try, past_trial, 4) == 0){
            printf("1\n");
            if(server_trial_num != trial_num){
                strcpy(response, "RTR DUP\n");
                is_duplicated = 1;
            }
            else is_last_duplicated = 1;
        }
    }

    if(trial_num != server_trial_num + 1){
        strcpy(response, "RTR INV\n");
        return;
    }

    if(is_duplicated) return;

    char code_copy[5], try_copy[5];
    strcpy(code_copy, code);
    strcpy(try_copy, try);
    number_blacks_and_whites(code_copy, try_copy, num_b_w);

    if(trial_num != 8 || num_b_w[0] == 4){
        sprintf(response, "RTR OK %d %d %d\n", trial_num, num_b_w[0], num_b_w[1]);
        if(num_b_w[0] == 4){
            //Fechar o jogo
        }
    }
    else{
        sprintf(response, "RTR ENT %c %c %c %c\n", code[0], code[1], code[2], code[3]);
        //Fechar o jogo
    }

    if(is_last_duplicated == 0){
        sprintf(line, "%d: %c%c%c%c %d %d %d\n",
        trial_num, try[0], try[1], try[2], try[3], num_b_w[0], num_b_w[1], trial_seconds);
        
        fseek(file, 0, SEEK_END);

        fprintf(file, "%s", line);
        fflush(file);
    }
    
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


