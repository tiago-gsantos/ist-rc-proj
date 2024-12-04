#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
  int fd;
  ssize_t n;
  char buffer[128];
  struct addrinfo hints,*res;
  int errcode;
  struct sockaddr_in addr;
  socklen_t addrlen;

  char host[NI_MAXHOST],service[NI_MAXSERV];
  
  fd = socket(AF_INET,SOCK_DGRAM,0); //UDP socket
  if(fd==-1) exit(1); //error

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET; //IPv4
  hints.ai_socktype=SOCK_DGRAM; //UDP socket
  
  errcode = getaddrinfo("193.136.138.14","58011",&hints,&res);
  if(errcode!=0) exit(1); //error
  
  n = sendto(fd,"Hello!\n",7,0,res->ai_addr,res->ai_addrlen);
  if(n==-1) exit(1); //error
  
  addrlen=sizeof(addr);
  n = recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
  if(n==-1)/*error*/exit(1);
  
  write(1,"echo: ",6);//stdout
  write(1,buffer,n);
  
  errcode=getnameinfo((struct sockaddr*)&addr,addrlen,host,sizeof host,service,sizeof service,0);
  if(errcode!=0)
    fprintf(stderr,"error: getnameinfo: %s\n", gai_strerror(errcode));
  else
    printf("sent by [%s:%s]\n",host,service);

  freeaddrinfo(res);
  close(fd);
  exit(0);
}