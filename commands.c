#include "commands.h"
#include <stdio.h>
#include <string.h>

#define WAIT_TIME_LIMIT 15
#define TRY_LIMIT 3


int cmd_start(char *request, unsigned int *player_id, int *trial_num, int fd_udp, struct addrinfo *res) {
    /*n = sendto(fd_udp,"Hello!\n",7,0,res->ai_addr,res->ai_addrlen);
    if(n==-1) exit(1); //error
    
    addrlen=sizeof(addr);
    n = recvfrom(fd_udp,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
    if(n==-1)/*errorexit(1);
    
    write(1,"echo: ",6);//stdout
    write(1,buffer,n)*/
  
    
    return 0;
}

//Fazer request através do res 