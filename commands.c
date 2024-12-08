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

#define WAIT_TIME_LIMIT 15
#define TRY_LIMIT 3
#define RESPONSE_LEN 32


int send_udp_request(char *request, int fd_udp, struct addrinfo *res, char *response) {
    
    int error = 1;
    ssize_t bytes_sent = sendto(fd_udp, request, strlen(request) * sizeof(char), 0, res->ai_addr,res->ai_addrlen);
    if(bytes_sent < 0) return 1;

    for (int tries = 0; tries < TRY_LIMIT ; tries++) {
        ssize_t bytes_received = recvfrom(fd_udp, response, RESPONSE_LEN, 0, NULL, NULL); // é preciso verificar addr de quem envia msg?
        
        if(bytes_received >= 0) {
            response[bytes_received] = '\0';
            error = 0;
            break;
        } 
    }

    if(error) return 1;

    return 0;
}

int send_tcp_request(char *request, int *fd_tcp, struct addrinfo *res, char *response) {
    int fd;
    ssize_t bytes_left, bytes_written, bytes_read;
    char *ptr;

    printf("%s", request);
  
    fd = socket(AF_INET,SOCK_STREAM,0); //TCP socket
    if(fd == -1) return 1; //error
  
    if(connect(fd, res->ai_addr, res->ai_addrlen) == -1) return 1; /*error*/

    bytes_left = (ssize_t) strlen(request);

    while(bytes_left > 0){
        bytes_written = write(fd, request, bytes_left);
        if(bytes_written < 0) return 1; /*error*/
        
        bytes_left -= bytes_written;
        request += bytes_written;
    }
    /*
    bytes_left
    while(bytes_left>0){
        bytes_read =read(fd,ptr,bytes_left);
    
        if(bytes_read == -1) exit(1);
        else if(bytes_read == 0) break; //closed by peer
        
        bytes_left-=bytes_read;
        ptr+=bytes_read;
    }
  
    nread=nbytes-nleft;
    
    
    close(fd);
    exit(0);
    */
    
    /*int error = 1;
    ssize_t bytes_sent = sendto(fd_udp, request, strlen(request) * sizeof(char), 0, res->ai_addr,res->ai_addrlen);
    if(bytes_sent < 0) return 1;

    for (int tries = 0; tries < TRY_LIMIT ; tries++) {
        ssize_t bytes_received = recvfrom(fd_udp, response, RESPONSE_LEN, 0, NULL, NULL); // é preciso verificar addr de quem envia msg?
        
        if(bytes_received >= 0) {
            response[bytes_received] = '\0';
            error = 0;
            break;
        } 
    }

    if(error) return 1;*/

    return 0;
}


void cmd_start(char *request, unsigned int *player_id, int *trial_num, int fd_udp, struct addrinfo *res) {
    char response[RESPONSE_LEN];
    char res_cmd[4];
    char status[4];
    char extra[1];
    
    if(send_udp_request(request, fd_udp, res, response) == 1) {
        fprintf(stderr, "Error while communicating with server!\n");
        close(fd_udp);
        exit(1);
    }

    if(sscanf(response, "%3s %3s %1s", res_cmd, status, extra) != 2){
        fprintf(stderr, "Invalid response from server!\n");
        return;
    }

    if(strcmp(res_cmd, "RSG") != 0){
        fprintf(stderr, "Invalid response from server!\n");
        return;
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

    return;
}


void cmd_try(char *request, int *trial_num, int fd_udp, struct addrinfo *res){
    char response[RESPONSE_LEN];
    char res_cmd[4];
    char status[4];
    char extra[1];
    unsigned int nt;
    unsigned int nb;
    unsigned int nw;
    char c[4];

    
    if(send_udp_request(request, fd_udp, res, response) == 1) {
        fprintf(stderr, "Error while communicating with server!\n");
        close(fd_udp);
        exit(1);
    }

    if(sscanf(response, "%3s %4s", res_cmd, status) != 2){
        fprintf(stderr, "Invalid response from server!\n");
        return;
    }

    if(strcmp(res_cmd, "RTR") != 0){
        fprintf(stderr, "Invalid response from server!\n");
        return;
    }

    if(strcmp(status, "OK") == 0){
        if(sscanf(response, "%*s %*s %u %u %u %1s", &nt, &nb, &nw, extra) != 3) {
            fprintf(stderr, "Invalid response from server!\n");
            return; 
        }
        if(nb == 4) {
            *trial_num = -1;
            printf("You win the game!\n");
        }
        else {
            (*trial_num)++;
            printf("Number of tries: %u\nNumber of blacks: %u\nNumber of whites: %u\n", nt, nb, nw);
        }
    }
    else if(strcmp(status, "DUP") == 0){
        printf("You repeted a previous guess. Please try a different guess.\n");
    }
    else if(strcmp(status, "INV") == 0){
        //*trial_num = ??
        printf("ERROR\n");
    }
    else if(strcmp(status, "NOK") == 0){
        fprintf(stderr, "Invalid command! Start a new game first.\n");
    }
    else if(strcmp(status, "ENT") == 0){
        if(sscanf(response, "%*s %*s %c %c %c %c %1s", &c[0], &c[1], &c[2], &c[3], extra) != 4) {
            fprintf(stderr, "Invalid response from server!\n");
            return; 
        }
        
        fprintf(stderr, "You lost! No more tries left. The color code was: %c %c %c %c\n", c[0], c[1], c[2], c[3]);
    }
    else if(strcmp(status, "ETM") == 0){
        if(sscanf(response, "%*s %*s %c %c %c %c %1s", &c[0], &c[1], &c[2], &c[3], extra) != 4) {
            fprintf(stderr, "Invalid response from server!\n");
            return; 
        }
        
        fprintf(stderr, "You lost! Time limit exceeded. The color code was: %c %c %c %c\n", c[0], c[1], c[2], c[3]);
    }
    else if(strcmp(status, "ERR") == 0){
        fprintf(stderr, "Invalid arguments!\n");
    }
    else{
        fprintf(stderr, "Invalid response from server!\n");
    }

    return;
}

void cmd_st(char *request, struct addrinfo *res){
    char response[RESPONSE_LEN];
    char res_cmd[4];
    char status[4];
    char extra[1];
    char c[4];

    if(send_tcp_request(request, res, response) == 1) {
        fprintf(stderr, "Error while communicating with server!\n");
        exit(1);
    }

    if(sscanf(response, "%3s %4s", res_cmd, status) != 2){
        fprintf(stderr, "Invalid response from server!\n");
        return;
    }
   
    if(strcmp(res_cmd, "RST") != 0){
        fprintf(stderr, "Invalid response from server!\n");
        return;
    }

    if(strcmp(status, "ACT") == 0){
        if(sscanf(response, "%*s %*s %c %c %c %c %1s", &c[0], &c[1], &c[2], &c[3], extra) != 4) {
            fprintf(stderr, "Invalid response from server!\n");
            return; 
        }

        fprintf(stderr, "You quit the game! The color code was: %c %c %c %c\n", c[0], c[1], c[2], c[3]);
    }
    else if(strcmp(status, "FIN") == 0){
        printf("Invalid command! You are not playing any game.\n");
        
    }
    else if(strcmp(status, "NOK") == 0){
        printf("Invalid command! You are not playing any game.\n");
        
    }
    else{
        fprintf(stderr, "Invalid response from server!\n");
    }

    return;
}


void cmd_sb(char *request, struct addrinfo *res){
    char response[RESPONSE_LEN];
    char res_cmd[4];
    char status[4];
    char extra[1];
    char c[4];

    
    if(send_udp_request(request, res, response) == 1) {
        fprintf(stderr, "Error while communicating with server!\n");
        exit(1);
    }

    /*if(sscanf(response, "%3s %4s", res_cmd, status) != 2){
        fprintf(stderr, "Invalid response from server!\n");
        return;
    }
   
    if(strcmp(res_cmd, "RQT") != 0){
        fprintf(stderr, "Invalid response from server!\n");
        return;
    }

    if(strcmp(status, "OK") == 0){
        if(sscanf(response, "%*s %*s %c %c %c %c %1s", &c[0], &c[1], &c[2], &c[3], extra) != 4) {
            fprintf(stderr, "Invalid response from server!\n");
            return; 
        }

        fprintf(stderr, "You quit the game! The color code was: %c %c %c %c\n", c[0], c[1], c[2], c[3]);
        *trial_num = -1;
    }
    else if(strcmp(status, "NOK") == 0){
        printf("Invalid command! You are not playing any game.\n");
        *trial_num = -1;
    }
    else if(strcmp(status, "ERR") == 0){
        printf("ERROR\n");
    }
    else{
        fprintf(stderr, "Invalid response from server!\n");
    }*/

    return;
}


void cmd_quit(char *request, int *trial_num, int fd_udp, struct addrinfo *res){
    char response[RESPONSE_LEN];
    char res_cmd[4];
    char status[4];
    char extra[1];
    char c[4];

    
    if(send_udp_request(request, fd_udp, res, response) == 1) {
        fprintf(stderr, "Error while communicating with server!\n");
        close(fd_udp);
        exit(1);
    }

    if(sscanf(response, "%3s %4s", res_cmd, status) != 2){
        fprintf(stderr, "Invalid response from server!\n");
        return;
    }
   
    if(strcmp(res_cmd, "RQT") != 0){
        fprintf(stderr, "Invalid response from server!\n");
        return;
    }

    if(strcmp(status, "OK") == 0){
        if(sscanf(response, "%*s %*s %c %c %c %c %1s", &c[0], &c[1], &c[2], &c[3], extra) != 4) {
            fprintf(stderr, "Invalid response from server!\n");
            return; 
        }

        fprintf(stderr, "You quit the game! The color code was: %c %c %c %c\n", c[0], c[1], c[2], c[3]);
        *trial_num = -1;
    }
    else if(strcmp(status, "NOK") == 0){
        printf("Invalid command! You are not playing any game.\n");
        *trial_num = -1;
    }
    else if(strcmp(status, "ERR") == 0){
        printf("ERROR\n");
    }
    else{
        fprintf(stderr, "Invalid response from server!\n");
    }

    return;
}


void cmd_debug(char *request, unsigned int *player_id, int *trial_num, int fd_udp, struct addrinfo *res) {
    char response[RESPONSE_LEN];
    char res_cmd[4];
    char status[4];
    char extra[1];

    
    if(send_udp_request(request, fd_udp, res, response) == 1) {
        fprintf(stderr, "Error while communicating with server!\n");
        close(fd_udp);
        exit(1);
    }

    if(sscanf(response, "%3s %3s %1s", res_cmd, status, extra) != 2){
        fprintf(stderr, "Invalid response from server!\n");
        return;
    }

    if(strcmp(res_cmd, "RDB") != 0){
        fprintf(stderr, "Invalid response from server!\n");
        return;
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

    return;
}


