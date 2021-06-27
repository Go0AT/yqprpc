#ifndef YQPRPC_H__
#define YQPRPC_H__

#include <map>
#include <functional>
#include <sys/epoll.h>
#include <unistd.h>
#include <set>
#include "serialize.h"
#include "proto.h"
#include "ipv4tcp.h"
#include "threadpool.h"
template<typename T>
struct type_xx{
    typedef T type;
};

template<>
struct type_xx<void>{
    typedef int8_t type;
};

template<typename T>
class val_t{
public:
    typedef typename type_xx<T>::type type;
private:
    type val;
    bool success;
public:
    type &getval();
    void setval(type &tmp);
    bool issuccess(){
        return success;
    }
    void setsuccess(bool val){
        success = val;
    }
};

template<typename T>
typename val_t<T>::type &val_t<T>::getval(){
    return val;
}

template<typename T>
void val_t<T>::setval(type &tmp){
    val = tmp;
}

class yqprpc{
private:
    std::map<std::string,std::function<void(serialize *,const char *,size_t)> > mp;
    std::thread serv;
    std::promise<void> serv_exit;
    std::future<void> promisefuture;
    ipv4tcp network;
    std::string serip;
    int sport;
    epoll_event events[10000];
    struct edata{
        int fd;
        serialize s;
    };
public:
    yqprpc();
    ~yqprpc() = default;
    void asclient(std::string serip,int port);
    void asserver(int port);
    void run();
    void servstart();
    void servclose();
    void settimeout(int timeout);

    template<typename F>
    void bind(std::string fname,F fun);
    template<typename F,typename C>
    void bind(std::string fname,F fun,C *c);
    //client
    template <typename R>
    val_t<R> call(message &msg);
    template <typename R>
    val_t<R> call(message &&msg);
private:
    serialize *call_(std::string fname,const char *data,size_t lens);
    template<typename R>
    val_t<R> netcall(serialize &s);

    template<typename F>
    void callproxy(F fun,serialize *s,const char *data,size_t lens);
    template<typename F,typename C>
    void callproxy(F fun,C *c,serialize *s,const char *data,size_t lens);

    template<typename R>
    void callproxy_(R (*fun)(const char*,size_t),serialize *s,const char *data,size_t lens);

    template<typename R,typename C>
    void callproxy_(R (C::*fun)(const char*,size_t),C *c,serialize *s,const char *data,size_t lens);

    template<typename R>
    void callproxy_(std::function<R(const char*,size_t)> fun,serialize *s,const char *data,size_t lens);
};

template<typename R,typename F>
typename std::enable_if<std::is_same<R,void>::value,typename type_xx<R>::type>::type callhelper(F fun){
    fun();
    return 0;
}

template<typename R,typename F>
typename std::enable_if<!std::is_same<R,void>::value,typename type_xx<R>::type>::type callhelper(F fun){
    return fun();
}

template<typename T,typename std::enable_if<class_serialize<typename type_xx<T>::type>::ret,T>::type * = nullptr>
uint32_t getserializesize(T &t){
    return t.serializesize();
}

template<typename T,typename std::enable_if<!class_serialize<typename type_xx<T>::type>::ret,T>::type * = nullptr>
uint32_t getserializesize(T &t){
    return sizeof(t);
}

template<>
uint32_t getserializesize(std::string &s);

template <typename R>
void yqprpc::callproxy_(std::function<R(const char*,size_t)> fun, serialize *s, const char *data, size_t lens){
    typename type_xx<R>::type ret = callhelper<R>(std::bind(fun,data,lens));
    uint32_t rlens = getserializesize(ret);
    // if(class_serialize<typename type_xx<R>::type>::ret) rlens = ret.serializesize();
    // else rlens = sizeof(ret);
    // rlens += sizeof(uint32_t);
    *s << (rlens + (uint32_t)sizeof(uint32_t)) << ret;
    return ;
}


template <typename F>
void yqprpc::callproxy(F fun, serialize *s, const char *data, size_t lens){
    callproxy_(fun,s,data,lens);
}

template <typename F,typename C>
void yqprpc::callproxy(F fun,C *c,serialize *s,const char *data,size_t lens){
    callproxy_(fun,c,s,data,lens);
}

template <typename R>
void yqprpc::callproxy_(R (*fun)(const char*,size_t), serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(const char *,size_t)>(fun),s,data,lens);
}


template <typename R, typename C>
void yqprpc::callproxy_(R (C::*fun)(const char*,size_t), C *c, serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(const char*,size_t)>(std::bind(fun,c,std::placeholders::_1,std::placeholders::_2)),s,data,lens);
}

template <typename F>
void yqprpc::bind(std::string fname, F fun){
    mp[fname] = std::bind(&yqprpc::callproxy<F>,this,fun,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
}

template <typename F,typename C>
void yqprpc::bind(std::string fname,F fun,C *c){
    mp[fname] = std::bind(&yqprpc::callproxy<F,C>,this,fun,c,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
}

template <typename R>
val_t<R> yqprpc::netcall(serialize &s){
    val_t<R> ret;
    ret.setsuccess(true);
    if(!network.mconnect(serip,sport)){
        ret.setsuccess(false);
        return ret;
    }
    if(!network.sendfuncmsg(s.getdata(),s.size())){
        ret.setsuccess(false);
        return ret;
    }
    streambuffer buf;
    if(!network.getret(buf)){
        ret.setsuccess(false);
        return ret;
    }
    // printf("getret!\n");
    serialize sret(buf);
    if(sret.size() < sizeof(uint32_t)){
        ret.setsuccess(false);
        return ret;
    }
    uint32_t lens;
    sret >> lens;
    if(sret.size() != lens){
        ret.setsuccess(false);
        return ret;
    }
    sret >> ret.getval();
    return ret;
}

template <typename R>
val_t<R> yqprpc::call(message &msg){
    serialize s;
    s << (uint32_t)(msg.serializesize() + (uint32_t)sizeof(uint32_t));
    msg.serialization(s);
    return netcall<R>(s);
}

template <typename R>
val_t<R> yqprpc::call(message &&msg){
    serialize s;
    s << ((uint32_t)(msg.serializesize()) + (uint32_t)sizeof(uint32_t));
    msg.serialization(s);
    return netcall<R>(s);
}

#endif