#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
  struct addrinfo hints,*res;
  int fd,errcode;
  struct sockaddr_in addr;
  socklen_t addrlen;
  ssize_t n;
  char buffer[128];

  fd = socket(AF_INET,SOCK_DGRAM,0); // UDP socket
  if(fd == -1) exit(1); /*error*/

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_DGRAM;//UDP socket
  hints.ai_flags=AI_PASSIVE;
  
  errcode = getaddrinfo(NULL,"58091",&hints,&res);
  if(errcode != 0) exit(1); /*error*/

  n = bind(fd,res->ai_addr,res->ai_addrlen);
  if(n == -1) exit(1); /*error*/

  while(1){
    addrlen = sizeof(addr);
    n = recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
    if(n == -1) exit(1); /*error*/

    write(1,"MSG: ",5);//stdout
    write(1,buffer,n);

    n = sendto(fd,"RSG OK\n",11,0,(struct sockaddr*)&addr,addrlen);
    if(n==-1) exit(1); /*error*/
  }
  
  //freeaddrinfo(res);
  //close(fd);
  //exit(0);
}
