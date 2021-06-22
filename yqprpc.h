#ifndef YQPRPC_H__
#define YQPRPC_H__

#include <map>
#include <functional>
#include "serialize.h"
#include "proto.h"
#include "ipv4tcp.h"
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
public:
    type getval();
    void setval(type &tmp);
};

template<typename T>
typename val_t<T>::type val_t<T>::getval(){
    return val;
}

template<typename T>
void val_t<T>::setval(type &tmp){
    val = tmp;
}

class yqprpc{
private:
    std::map<std::string,std::function<void(serialize *,const char *,size_t)> > mp;
    ipv4tcp network;
    std::string serip;
    int sport;
public:
    yqprpc() = default;
    ~yqprpc() = default;
    void asclient(std::string serip,int port);
    void asserver(int port);
    void run();

    template<typename F>
    void bind(std::string fname,F fun);
    template<typename F,typename C>
    void bind(std::string fname,F fun,C *c);
    //client
    template <typename R>
    R call(message &msg);
    template <typename R>
    R call(message &&msg);
private:
    serialize *call_(std::string fname,const char *data,size_t lens);
    template<typename R>
    R netcall(serialize &s);

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

template <typename R>
void yqprpc::callproxy_(std::function<R(const char*,size_t)> fun, serialize *s, const char *data, size_t lens){
    typename type_xx<R>::type ret = callhelper<R>(std::bind(fun,data,lens));
    *s << ret;
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
R yqprpc::netcall(serialize &s){
    network.mconnect(serip,sport);
    network.sendfuncmsg(s.getdata(),s.size());
    streambuffer buf;
    network.getret(buf);
    // printf("getret!\n");
    serialize sret(buf);
    typename type_xx<R>::type ret;
    sret >> ret;
    return (R)ret;
}

template <typename R>
R yqprpc::call(message &msg){
    serialize s;
    msg.serialization(s);
    return netcall<R>(s);
}

template <typename R>
R yqprpc::call(message &&msg){
    serialize s;
    msg.serialization(s);
    return netcall<R>(s);
}

#endif