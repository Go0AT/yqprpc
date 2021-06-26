#ifndef IPV4TCP_H__
#define IPV4TCP_H__

#include <string>
#include <fcntl.h>
#include <signal.h>
#include "serialize.h"
class ipv4tcp{
private:
    int listensd;
    int connectsd;
    bool open;
    bool writen(int sd,const char *s,size_t lens);
public:
    ipv4tcp();
    ~ipv4tcp();
    void mlisten(int port);
    bool mconnect(std::string serip,int port);
    void getfuncmsg(streambuffer &buf);
    bool sendfuncmsg(const char *s,size_t lens);
    bool getret(streambuffer &buf);
    void sendret(const char *s,size_t lens);
    int getconnectsd();
    int getlistensd();
    static void setnoblock(int sd);
};

#endif