#ifndef YQPRPC_H__
#define YQPRPC_H__

#include <map>
#include <functional>
#include "serialize.h"
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
    template<typename R>
    R call(std::string fname);
    template<typename R,typename P1>
    R call(std::string fname,P1);
    template<typename R,typename P1,typename P2>
    R call(std::string fname,P1,P2);
    template<typename R,typename P1,typename P2,typename P3>
    R call(std::string fname,P1,P2,P3);
    template<typename R,typename P1,typename P2,typename P3,typename P4>
    R call(std::string fname,P1,P2,P3,P4);
    template<typename R,typename P1,typename P2,typename P3,typename P4,typename P5>
    R call(std::string fname,P1,P2,P3,P4,P5);
private:
    serialize *call_(std::string fname,const char *data,size_t lens);
    template<typename R>
    R netcall(serialize &s);

    template<typename F>
    void callproxy(F fun,serialize *s,const char *data,size_t lens);
    template<typename F,typename C>
    void callproxy(F fun,C *c,serialize *s,const char *data,size_t lens);
    template<typename R>
    void callproxy_(R (*fun)(),serialize *s,const char *data,size_t lens);
    template<typename R,typename P1>
    void callproxy_(R (*fun)(P1),serialize *s,const char *data,size_t lens);
    template<typename R,typename P1,typename P2>
    void callproxy_(R (*fun)(P1,P2),serialize *s,const char *data,size_t lens);
    template<typename R,typename P1,typename P2,typename P3>
    void callproxy_(R (*fun)(P1,P2,P3),serialize *s,const char *data,size_t lens);
    template<typename R,typename P1,typename P2,typename P3,typename P4>
    void callproxy_(R (*fun)(P1,P2,P3,P4),serialize *s,const char *data,size_t lens);
    template<typename R,typename P1,typename P2,typename P3,typename P4,typename P5>
    void callproxy_(R (*fun)(P1,P2,P3,P4,P5),serialize *s,const char *data,size_t lens);

    template<typename R,typename C>
    void callproxy_(R (C::*fun)(),C *c,serialize *s,const char *data,size_t lens);
    template<typename R,typename C,typename P1>
    void callproxy_(R (C::*fun)(P1),C *c,serialize *s,const char *data,size_t lens);
    template<typename R,typename C,typename P1,typename P2>
    void callproxy_(R (C::*fun)(P1,P2),C *c,serialize *s,const char *data,size_t lens);
    template<typename R,typename C,typename P1,typename P2,typename P3>
    void callproxy_(R (C::*fun)(P1,P2,P3),C *c,serialize *s,const char *data,size_t lens);
    template<typename R,typename C,typename P1,typename P2,typename P3,typename P4>
    void callproxy_(R (C::*fun)(P1,P2,P3,P4),C *c,serialize *s,const char *data,size_t lens);
    template<typename R,typename C,typename P1,typename P2,typename P3,typename P4,typename P5>
    void callproxy_(R (C::*fun)(P1,P2,P3,P4,P5),C *c,serialize *s,const char *data,size_t lens);

    template<typename R>
    void callproxy_(std::function<R()> fun,serialize *s,const char *data,size_t lens);
    template<typename R,typename P1>
    void callproxy_(std::function<R(P1)> fun,serialize *s,const char *data,size_t lens);
    template<typename R,typename P1,typename P2>
    void callproxy_(std::function<R(P1,P2)> fun,serialize *s,const char *data,size_t lens);
    template<typename R,typename P1,typename P2,typename P3>
    void callproxy_(std::function<R(P1,P2,P3)> fun,serialize *s,const char *data,size_t lens);
    template<typename R, typename P1, typename P2, typename P3,typename P4>
    void callproxy_(std::function<R(P1, P2, P3,P4)> fun, serialize *s, const char *data, size_t lens);
    template<typename R,typename P1,typename P2,typename P3,typename P4,typename P5>
    void callproxy_(std::function<R(P1,P2,P3,P4,P5)> fun,serialize *s,const char *data,size_t lens);
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
void yqprpc::callproxy_(std::function<R()> fun, serialize *s, const char *data, size_t lens){
    typename type_xx<R>::type ret = callhelper<R>(fun);
    *s << ret;
    return ;
}
template <typename R, typename P1>
void yqprpc::callproxy_(std::function<R(P1)> fun, serialize *s, const char *data, size_t lens){
    serialize getp(streambuffer(data,lens));
    P1 p1;
    getp >> p1;
    typename type_xx<R>::type ret = callhelper<R>(std::bind(fun,p1));
    *s << ret;
    return ;
}
template <typename R, typename P1, typename P2>
void yqprpc::callproxy_(std::function<R(P1, P2)> fun, serialize *s, const char *data, size_t lens){
    serialize getp(streambuffer(data,lens));
    P1 p1; P2 p2;
    getp >> p1 >> p2;
    typename type_xx<R>::type ret = callhelper<R>(std::bind(fun,p1,p2));
    *s << ret;
    return ;
}
template <typename R, typename P1, typename P2, typename P3>
void yqprpc::callproxy_(std::function<R(P1, P2, P3)> fun, serialize *s, const char *data, size_t lens){
    serialize getp(streambuffer(data,lens));
    P1 p1; P2 p2; P3 p3;
    getp >> p1 >> p2 >> p3;
    typename type_xx<R>::type ret = callhelper<R>(std::bind(fun,p1,p2,p3));
    *s << ret;
    return ;
}
template <typename R, typename P1, typename P2, typename P3, typename P4>
void yqprpc::callproxy_(std::function<R(P1, P2, P3, P4)> fun, serialize *s, const char *data, size_t lens){
    serialize getp(streambuffer(data,lens));
    P1 p1; P2 p2; P3 p3; P4 p4;
    getp >> p1 >> p2 >> p3 >> p4;
    typename type_xx<R>::type ret = callhelper<R>(std::bind(fun,p1,p2,p3,p4));
    *s << ret;
    return ;
}
template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
void yqprpc::callproxy_(std::function<R(P1, P2, P3, P4, P5)> fun, serialize *s, const char *data, size_t lens){
    serialize getp(streambuffer(data,lens));
    P1 p1; P2 p2; P3 p3; P4 p4; P5 p5;
    getp >> p1 >> p2 >> p3 >> p4 >> p5;
    typename type_xx<R>::type ret = callhelper<R>(std::bind(fun,p1,p2,p3,p4,p5));
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
void yqprpc::callproxy_(R (*fun)(), serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R()>(fun),s,data,lens);
}
template <typename R, typename P1>
void yqprpc::callproxy_(R (*fun)(P1), serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(P1)>(fun),s,data,lens);
}
template <typename R, typename P1, typename P2>
void yqprpc::callproxy_(R (*fun)(P1, P2), serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(P1,P2)>(fun),s,data,lens);
}
template <typename R, typename P1, typename P2, typename P3>
void yqprpc::callproxy_(R (*fun)(P1, P2, P3), serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(P1,P2,P3)>(fun),s,data,lens);
}
template <typename R, typename P1, typename P2, typename P3, typename P4>
void yqprpc::callproxy_(R (*fun)(P1, P2, P3, P4), serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(P1,P2,P3,P4)>(fun),s,data,lens);
}
template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
void yqprpc::callproxy_(R (*fun)(P1, P2, P3, P4, P5), serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(P1,P2,P3,P4,P5)>(fun),s,data,lens);
}

template <typename R, typename C>
void yqprpc::callproxy_(R (C::*fun)(), C *c, serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R()>(std::bind(fun,c)),s,data,lens);
}
template <typename R, typename C, typename P1>
void yqprpc::callproxy_(R (C::*fun)(P1), C *c, serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(P1)>(std::bind(fun,c,std::placeholders::_1)),s,data,lens);
}
template <typename R, typename C, typename P1, typename P2>
void yqprpc::callproxy_(R (C::*fun)(P1, P2), C *c, serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(P1,P2)>(std::bind(fun,c,std::placeholders::_1,std::placeholders::_2)),s,data,lens);
}
template <typename R, typename C, typename P1, typename P2, typename P3>
void yqprpc::callproxy_(R (C::*fun)(P1, P2, P3), C *c, serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(P1,P2,P3)>(std::bind(fun,c,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)),
    s,data,lens);
}
template <typename R, typename C, typename P1, typename P2, typename P3, typename P4>
void callproxy_(R (C::*fun)(P1, P2, P3, P4), C *c, serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(P1,P2,P3,P4)>(std::bind(fun,c,std::placeholders::_1,std::placeholders::_2,
    std::placeholders::_3,std::placeholders::_4)),s,data,lens);
}
template <typename R, typename C, typename P1, typename P2, typename P3, typename P4, typename P5>
void callproxy_(R (C::*fun)(P1, P2, P3, P4, P5), C *c, serialize *s, const char *data, size_t lens){
    callproxy_(std::function<R(P1,P2,P3,P4,P5)>(std::bind(fun,c,std::placeholders::_1,std::placeholders::_2,
    std::placeholders::_3,std::placeholders::_4,std::placeholders::_5)),s,data,lens);
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
R yqprpc::call(std::string fname){
    serialize s;
    s << fname;
    return netcall<R>(s);
}
template <typename R, typename P1>
R yqprpc::call(std::string fname, P1 p1){
    serialize s;
    s << fname << p1;
    return netcall<R>(s);
}
template <typename R, typename P1, typename P2>
R yqprpc::call(std::string fname, P1 p1, P2 p2){
    serialize s;
    s << fname << p1 << p2;
    return netcall<R>(s);
}
template <typename R, typename P1, typename P2, typename P3>
R yqprpc::call(std::string fname, P1 p1, P2 p2, P3 p3){
    serialize s;
    s << fname << p1 << p2 << p3;
    return netcall<R>(s);
}
template <typename R, typename P1, typename P2, typename P3, typename P4>
R yqprpc::call(std::string fname, P1 p1, P2 p2, P3 p3, P4 p4){
    serialize s;
    s << fname << p1 << p2 << p3 << p4;
    return netcall<R>(s);
}
template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
R yqprpc::call(std::string fname, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5){
    serialize s;
    s << fname << p1 << p2 << p3 << p4 << p5;
    return netcall<R>(s);
}

#endif