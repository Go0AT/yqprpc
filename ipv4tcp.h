#ifndef IPV4TCP_H__
#define IPV4TCP_H__

#include <string>
#include "serialize.h"
class ipv4tcp{
private:
    int listensd;
    int connectsd;
    bool open;
    void writen(int sd,const char *s,size_t lens);
public:
    ipv4tcp();
    ~ipv4tcp();
    void mlisten(int port);
    void mconnect(std::string serip,int port);
    void getfuncmsg(streambuffer &buf);
    void sendfuncmsg(const char *s,size_t lens);
    void getret(streambuffer &buf);
    void sendret(const char *s,size_t lens);
    int getconnectsd();
};

#endif