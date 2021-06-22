
#include "yqprpc.h"
#include <cstdio>


void yqprpc::asclient(std::string serip, int port){
    this->serip = serip;
    this->sport = port;
}

void yqprpc::asserver(int port){
    network.mlisten(port);
}

serialize *yqprpc::call_(std::string fname, const char *data, size_t lens){
    serialize *s = new serialize;
    if(!mp.count(fname)){
        printf("no bind!\n");
        return s;
    }
    auto fun = mp[fname];
    fun(s,data,lens);
    return s;
}

void yqprpc::run(){
    printf("start to run\n");
    while(true){
        streambuffer buf;
        network.getfuncmsg(buf);
        // printf("getfuncmsg!\n");
        serialize s(buf);
        std::string fname;
        s >> fname;
        serialize *ret = call_(fname,s.getcurrentdata(),s.size() - fname.size() - sizeof(uint32_t));
        network.sendret(ret->getdata(),ret->size());
        delete ret;
    }
}
