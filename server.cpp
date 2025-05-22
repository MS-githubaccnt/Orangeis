#include <sys/socket.h>
#include <netinet/in.h>
#include  <netinet/ip.h>
#include <sys/types.h> 
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <cstdlib>
#include <cassert>
#include <cerrno>
void die(char* a){
    perror(a);
    exit(1);
}
//reaidng and writing n bytes from/to a socket using TCP be like
static int32_t read_full(int fd,char *buf, size_t n){
    while(n>0){
        ssize_t rv=read(fd,buf,n);
        if(rv<=0){
            return -1;
        }
        assert((size_t)rv<=n);
        n-=(size_t)rv;
        buf+=rv;
    }
    return 0;
}
static int32_t write_all(int fd, const char *buf, size_t n){
    while(n>0){
            ssize_t rv=write(fd,buf,n);
            if(rv<=0){
                return -1;
            }
            assert((size_t)rv<=n);
            n-=(size_t)rv;
            buf+=rv;
    }
    return 0;
}
const size_t k_max_msg=4096;
static int32_t one_request(int connfd){
    char rbuf[4+k_max_msg];//4 is reserved for the header
    errno=0;// clearing old errors
    int32_t err=read_full(connfd,rbuf,4);
    if(err){
        perror(errno==0?"EOF":"read() error");
        return err;
    }
    uint32_t len=0;
    memcpy(&len,rbuf,4);//assume lil endian
    if(len>k_max_msg){
        perror("tooooo loooong");
        return -1;
    }
    err=read_full(connfd,&rbuf[4],len);
    if(err){
        perror("read() error");
        return err;
    }
    printf ("client says: %.*s\n",len,&rbuf[4]);
    //reply time
    const char reply[]="world";
    char wbuf[4+sizeof(reply)];
    len=(uint32_t)strlen(reply);
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4],reply,len);
    return write_all(connfd,wbuf,4+len);
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
    while(true){
        int32_t err=one_request(connfd);
        if(err){
            break;
        }
    }
    //do_something(connfd);
    close(connfd);
}
}