#include "ipv4tcp.h"
#include "byteorder.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <mutex>
#include <assert.h>
// static std::mutex mut;

ipv4tcp::ipv4tcp(){
    timeout = 75;
    open = false;
}

ipv4tcp::~ipv4tcp(){
    if(open) close(listensd);
}

void ipv4tcp::settimeout(int timeout){
    this->timeout = timeout;
}

void ipv4tcp::mlisten(int port){
    listensd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    sockaddr_in laddr;
    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(port);
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);
    socklen_t len = sizeof(sockaddr_in);
    bind(listensd,(sockaddr *)&laddr,len);
    listen(listensd,5000);
    open = true;
    setnoblock(listensd);
}

bool ipv4tcp::mconnect(std::string serip, int port){
    // printf("haha\n");
    connectsd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    inet_pton(AF_INET,serip.c_str(),&saddr.sin_addr);
    socklen_t len = sizeof(sockaddr_in);
    // printf("con\n");
    int flags = fcntl(connectsd,F_GETFL);
    fcntl(connectsd,F_SETFL,flags | O_NONBLOCK);
    int n;
    if((n = connect(connectsd,(sockaddr *)&saddr,len)) < 0){
        // perror("connect()");
        if(errno != EINPROGRESS){
            // perror("connect()");
            close(connectsd);
            return false;
        }
    }
    // printf("---n = %d\n",n);
    if(n == 0){
        // printf("***\n");
        fcntl(connectsd,F_SETFL,flags);
        return true;
    }
    fd_set rset,wset;
    FD_ZERO(&rset);
    FD_SET(connectsd,&rset);
    wset = rset;
    timeval tv;
    // printf("timeout = %d\n",timeout);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    if((n = select(connectsd + 1,&rset,&wset,NULL,&tv)) == 0){
        // printf("???\n");
        close(connectsd);
        return false;
    }
    if(FD_ISSET(connectsd,&rset) || FD_ISSET(connectsd,&wset)){
        // n = connect(connectsd,(sockaddr*)&saddr,len);
        // int ret = read(connectsd,&n,0);
        // printf("ret = %d\n",ret);
        // if(errno != EISCONN){
        //     close(connectsd);
        //     return false;
        // }
        // if(ret < 0){
        //     close(connectsd);
        //     return false;
        // }
        int error;
        socklen_t lens = sizeof(error);
        getsockopt(connectsd,SOL_SOCKET,SO_ERROR,&error,&lens);
        if(error){
            errno = error;
            // perror("getsockopt()");
            close(connectsd);
            return false;
        }
    }
    fcntl(connectsd,F_SETFL,flags);
    
    // printf("conf\n");
    return true;
}

bool ipv4tcp::writen(int sd,const char *s,size_t lens){
    int pos = 0;
    while(true){
        ssize_t ret = write(sd,s + pos,lens - pos);
        if(ret < 0){
            perror("write()");
            return false;
        }
        pos += ret;
        if(pos == lens) break;
    }
    return true;
}

void ipv4tcp::getfuncmsg(streambuffer &buf){
    connectsd = accept(listensd,NULL,NULL);
    char arr[100];
    while(true){
        ssize_t lens = read(connectsd,arr,100);
        if(lens < 0){
            perror("read()");
            continue;
        }
        if(!lens) break;
        for(size_t i = 0 ; i < lens ; i++ )
            buf.emplace_back(arr[i]);
    }
}
bool ipv4tcp::sendfuncmsg(const char *s,size_t lens){
    // printf("write\n");
    if(!writen(connectsd,s,lens)){
        close(connectsd);
        return false;
    }
    // printf("writef\n");
    shutdown(connectsd,SHUT_WR);
    return true;
}
bool ipv4tcp::getret(streambuffer &buf){
    char arr[100];
    while(true){
        ssize_t lens = read(connectsd,arr,100);
        if(lens < 0){
            // perror("ret read()");
            close(connectsd);
            return false;
        }
        if(!lens) break;
        // for(size_t i = 0 ; i < lens ; i++ )
        //     buf.
        // assert(lens <= 1000);
        buf.input(arr,lens);
    }
    close(connectsd);
    return true;
}
void ipv4tcp::sendret(const char *s,size_t lens){
    writen(connectsd,s,lens);
    close(connectsd);
}

int ipv4tcp::getconnectsd(){
    return connectsd;
}

int ipv4tcp::getlistensd(){
    return listensd;
}

void ipv4tcp::setnoblock(int sd){
    int flags = fcntl(sd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(sd, F_SETFL, flags);
}