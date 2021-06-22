#include "ipv4tcp.h"
#include "byteorder.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
// #include <fcntl.h>

ipv4tcp::ipv4tcp(){
    open = false;
}

ipv4tcp::~ipv4tcp(){
    if(open) close(listensd);
}

void ipv4tcp::mlisten(int port){
    listensd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    sockaddr_in laddr;
    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(port);
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);
    socklen_t len = sizeof(sockaddr_in);
    bind(listensd,(sockaddr *)&laddr,len);
    listen(listensd,10);
    open = true;
    // int flags = fcntl(listensd,F_GETFL);
    // flags |= O_NONBLOCK;
    // fcntl(listensd,F_SETFL,flags);
}

void ipv4tcp::mconnect(std::string serip, int port){
    connectsd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    inet_pton(AF_INET,serip.c_str(),&saddr.sin_addr);
    socklen_t len = sizeof(sockaddr_in);
    connect(connectsd,(sockaddr *)&saddr,len);
}

void ipv4tcp::writen(int sd,const char *s,size_t lens){
    int pos = 0;
    while(true){
        size_t ret = write(sd,s + pos,lens - pos);
        if(ret < 0){
            perror("write()");
            continue;
        }
        pos += ret;
        if(pos == lens) break;
    }
}

void ipv4tcp::getfuncmsg(streambuffer &buf){
    connectsd = accept(listensd,NULL,NULL);
    char arr[100];
    while(true){
        size_t lens = read(connectsd,arr,100);
        if(lens < 0){
            perror("read()");
            continue;
        }
        if(!lens) break;
        for(size_t i = 0 ; i < lens ; i++ )
            buf.emplace_back(arr[i]);
    }
}
void ipv4tcp::sendfuncmsg(const char *s,size_t lens){
    writen(connectsd,s,lens);
    shutdown(connectsd,SHUT_WR);
}
void ipv4tcp::getret(streambuffer &buf){
    char arr[100];
    while(true){
        size_t lens = read(connectsd,arr,100);
        if(lens < 0){
            perror("read()");
            continue;
        }
        if(!lens) break;
        for(size_t i = 0 ; i < lens ; i++ )
            buf.emplace_back(arr[i]);
    }
    close(connectsd);
}
void ipv4tcp::sendret(const char *s,size_t lens){
    writen(connectsd,s,lens);
    close(connectsd);
}

int ipv4tcp::getconnectsd(){
    return connectsd;
}