#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  

#define PORT 8800
#define REMOTE_IP "127.0.0.1"

int main(int argc, char *argv[])  
{  
    int fd;  
    int len;  
    struct sockaddr_in remote_addr;
    char buf[BUFSIZ] = "hello tcp!";
	
    if(0 > (fd=socket(PF_INET, SOCK_STREAM, 0)))  
    {  
        perror("socket");  
        return -1;
    }  
     
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET; 
    remote_addr.sin_addr.s_addr = inet_addr(REMOTE_IP);
    remote_addr.sin_port = htons(PORT);
	
    if(0 > connect(fd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)))  
    {  
        perror("connect");  
        return -1;  
    }  

    printf("Start client ... \n");
    while(1)  
    {  
        len=send(fd, buf, strlen(buf), 0);  
        //len=recv(fd, buf, BUFSIZ, 0);  
        //buf[len] = '\0';  
        //printf("recv: %s\n", buf);
	   sleep(1); 
    }
  
    close(fd);
    return 0;  
}  
