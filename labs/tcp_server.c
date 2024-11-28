#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void)
  {
  struct addrinfo hints,*res;
  int fd,newfd,errcode;
  ssize_t n, nw;
  struct sockaddr_in addr;
  socklen_t addrlen;
  char *ptr,buffer[128];
  
  fd = socket(AF_INET,SOCK_STREAM,0); // TCP socket
  if(fd == -1) exit(1); //error

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket
  hints.ai_flags=AI_PASSIVE;
  
  errcode = getaddrinfo(NULL,"58001",&hints,&res);
  if(errcode != 0) exit(1); // error

  n = bind(fd,res->ai_addr,res->ai_addrlen);
  if(n == -1) exit(1); // error

  if(listen(fd, 5) == -1) exit(1); // error

  while(1){
    addrlen=sizeof(addr);
    
    newfd = accept(fd,(struct sockaddr*)&addr,&addrlen);
    if(newfd == -1) exit(1); // error

    while((n = read(newfd,buffer,128))!=0){
      if(n==-1)/*error*/exit(1);
      ptr=&buffer[0];
      while(n>0){
        nw=write(newfd,ptr,n);
        if(nw <= 0)/*error*/exit(1);
        n-=nw; 
        ptr+=nw;
      }
      write(1,buffer,nw); // stdout
    }
    close(newfd);
  }
  //freeaddrinfo(res);
  //close(fd);
  //exit(0);
}