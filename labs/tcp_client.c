#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
  struct addrinfo hints,*res;
  int fd,n;
  ssize_t nbytes,nleft,nwritten,nread;
  char *ptr,buffer[128];

  
  fd = socket(AF_INET,SOCK_STREAM,0); //TCP socket
  if(fd==-1)exit(1); //error
  
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket
  
  n = getaddrinfo("LAPTOP-R9E7VMSR","58001",&hints,&res);
  if(n!=0) exit(1); /*error*/
  
  n = connect(fd,res->ai_addr,res->ai_addrlen);
  if(n==-1) exit(1); /*error*/

  ptr = strcpy(buffer,"Hello!\n");
  nbytes = 7;
  nleft = nbytes;

  while(nleft>0){
    nwritten = write(fd,ptr,nleft);
    if(nwritten<=0) exit(1); /*error*/
    
    nleft-=nwritten;
    ptr+=nwritten;
  }
    
  nleft=nbytes; 
  ptr=buffer;
  
  while(nleft>0){
    nread=read(fd,ptr,nleft);
    
    if(nread==-1) exit(1); /*error*/
    else if(nread==0)break;//closed by peer
    
    nleft-=nread;
    ptr+=nread;
  }
  
  nread=nbytes-nleft;
  
  write(1,"echo: ",6);//stdout
  write(1,buffer,nread);
  
  freeaddrinfo(res);
  close(fd);
  exit(0);

}