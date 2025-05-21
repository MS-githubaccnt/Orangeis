#include <sys/socket.h>
#include <netinet/in.h>
#include  <netinet/ip.h>
#include <sys/types.h> 
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <cstdlib>
void die(char* a){
    perror(a);
    exit(1);
}
static void do_something(int connfd){
    char rbuf[64]={};
    ssize_t n=read(connfd, rbuf, sizeof(rbuf)-1);
    if (n<0){
        perror("read() error");
        return;
    }
    printf("client says: %s\n", rbuf);
    char wbuf[]="world";
    write(connfd, wbuf, strlen(wbuf));
}
int main(){
int fd=socket (AF_INET,SOCK_STREAM,0);
if(fd<0){
    die("socket()");
}
//fd is the file descriptor for an ipv4 socket which uses tcp
int val=1;
int res=setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
struct sockaddr_in addr={};
addr.sin_family=AF_INET;
//htns and htonl convert b/w endians. CONVERT B/W 2 ENDIANS IS SYMMETRIC IE CONVERT B/W 2 NOT FROM 1 TO THE OTHER
addr.sin_port=htons(1234);
addr.sin_addr.s_addr=htonl(0);
//addr len is taken because different sizes for ipv4 and ipv6
int rv=bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
if(rv){die("bind()");}
rv=listen(fd,SOMAXCONN);
if(rv){die("listen()");}
while (true){
    struct sockaddr_in client_addr={};
    socklen_t addrlen=sizeof(client_addr);
    int connfd=accept(fd, (struct sockaddr *)&client_addr, &addrlen);
    if (connfd<0){
        continue;
    }
    do_something(connfd);
    close(connfd);
}
}