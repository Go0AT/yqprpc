# yqprpc

可以定义自己想要当作参数传输的对象，把对象声明放在transobject.h中，需要实现 serialize &serialization(serialize &s)和serialize &deserialization(serialize &s)和size_t serializesize()方法，客户端在使用call<>()时，需要传入调用远程函数需要的参数类(继承自message虚基类(放于proto.h中)，需要实现virtual serialize &serialization(serialize &s)方法以及virtual size_t serializesize()方法)，参数类存放于funparam.h中，使用参数类可以使服务器bind的可被远程调用的函数所需要的参数的数量不受限制(把需要调用的函数用新函数包装起来，相当于让用户解决了这个问题)(之前想过用C++ 11泛型解决，还没想到解决办法)。框架使用函数名作为键，所以若函数同名，则会取消先前绑定的同名函数。使用本框架时，可以通过函数返回值val_t类型的issuccess()方法查看远程调用是否成功，val_t中还有原本函数调用的返回值(可以设置client调用connect调用的超时时间(以秒为单位)，默认值为75s)，本框架使用C++ 11编译。

#### 性能测试
对于下面的例子，如果客户端不调用stdio的话，那么客户端发出6000(1000并发，每个线程请求6次)个请求且得到结果大约耗时0.7s,如果将得到的返回值打印出来，那么耗时0.9s~1.0s，如果发现服务器丢弃了某些连接，一个原因时listen()函数的第二个参数设置过小，另一个原因是内核给出的半连接队列和已连接队列较小，需要扩大/proc/sys/net/ipv4/tcp_max_syn_backlog和/proc/sys/net/core/somaxconn的数值。
#### 小例子

#### Server

```C++
#include "yqprpc.h"
#include "funparam.h"
#include <iostream>
#include <algorithm>
int test(const char *data,size_t lens){
    int a,b,c;
    serialize s(streambuffer(data,lens));
    s >> a >> b >> c;
    return a + b + c;
}

void test2(const char *data,size_t lens){
    std::string str;
    serialize s(streambuffer(data,lens));
    s >> str;
    std::cout << str << std::endl;
}

std::string test3(const char *data,size_t lens){
    std::string str;
    serialize s(streambuffer(data,lens));
    s >> str;
    std::reverse(str.begin(),str.end());
    return str;
}

class A{
public:
    int a;
    void test4(const char *data,size_t lens){
        std::cout << a << std::endl;
    }
};

void test5(const char *data,size_t lens){
    valtype tmp;
    serialize s(streambuffer(data,lens));
    s >> tmp;
    std::cout << tmp.a << std::endl;
    std::cout << tmp.b << std::endl;
}

double test6(const char *data,size_t lens){
    double a;
    serialize s(streambuffer(data,lens));
    s >> a;
    return --a;
}

A tmp;

int main(){
    yqprpc server;
    server.asserver(2021);
    server.bind("test",test);
    server.bind("test2",test2);
    server.bind("test3",test3);
    tmp.a = 789;
    server.bind("test4",&A::test4,&tmp);
    server.bind("test5",test5);
    server.bind("test6",test6);
    server.servstart();
    int tmp; scanf("%d",&tmp);
    server.servclose();
    return 0;
}
```

#### Client

```C++
#include "yqprpc.h"
#include "funparam.h"
#include <vector>
#include <cstdio>
#include <iostream>
void test(){
    yqprpc client;
    client.asclient("127.0.0.1", 2021);
    client.settimeout(5);
    auto ret = client.call<int>(msgtest("test", 1, 2, 3));
    if (ret.issuccess()) printf("ret = %d\n", ret);
    else printf("error\n");
    std::string str("123");
    if(!client.call<void>(msgtest2("test2", str)).issuccess()) printf("error\n");
    auto ret3 = client.call<std::string>(msgtest3("test3", str));
    if (ret3.issuccess()) std::cout << ret3.getval() << std::endl;
    else printf("error\n");
    if(!client.call<void>(msgtest4("test4")).issuccess()) printf("error\n");
    valtype tmp;
    tmp.a = 456;
    tmp.b = 654;
    if(!client.call<void>(msgtest5("test5", tmp)).issuccess()) printf("error\n");
    auto test6 = client.call<double>(msgtest6("test6", 3.14));
    if (test6.issuccess()) std::cout << test6.getval() << std::endl;
    else printf("error\n");
    return ;
}

int main(){
    std::vector<std::thread> vec(1000);
    for(int i = 0 ; i < 1000 ; i++ )
        vec[i] = std::thread(test);
    for(int i = 0 ; i < 1000 ; i++ )
        vec[i].join();
    return 0;
}
```

#### Proto

```C++
#ifndef PROTO_H__
#define PROTO_H__

#include "serialize.h"

class message{
private:
    std::string fname;
public:
    message(std::string s): fname(s){}
    std::string &getfname(){
        return fname;
    }
    virtual serialize &serialization(serialize &s) = 0;
    virtual size_t serializesize() = 0;
};


#endif
```

#### Transobject.h

```C++
#ifndef TRANSOBJECT_H__
#define TRANSOBJECT_H__

#include "serialize.h"
class valtype{
public:
    int a, b;
    serialize &serialization(serialize &s){
        s << a << b;
        return s;
    }
    serialize &deserialization(serialize &s){
        s >> a >> b;
        return s;
    }
    size_t serializesize(){
        return sizeof(a) + sizeof(b);
    }
};

#endif
```

#### Funparam.h

```C++
#ifndef FUNPARAM_H__
#define FUNPARAM_H__

#include "proto.h"
#include "transobject.h"
class msgtest : public message{
private:
    int a, b, c;

public:
    msgtest(std::string s, int a, int b, int c) : message(s), a(a), b(b), c(c) {}
    virtual serialize &serialization(serialize &s){
        s << getfname() << a << b << c;
        return s;
    }
    virtual size_t serializesize(){
        return sizeof(uint32_t) + getfname().size() + sizeof(a) + sizeof(b) + sizeof(c);
    }
};


class msgtest2 : public message{
private:
    std::string str;

public:
    msgtest2(std::string s, std::string str) : message(s), str(str) {}
    virtual serialize &serialization(serialize &s){
        s << getfname() << str;
        return s;
    }
    virtual size_t serializesize(){
        return 2 * sizeof(uint32_t) + getfname().size() + str.size();
    }
};

class msgtest3 : public message{
private:
    std::string str;

public:
    msgtest3(std::string s, std::string str) : message(s), str(str) {}
    virtual serialize &serialization(serialize &s){
        s << getfname() << str;
        return s;
    }
    virtual size_t serializesize(){
        return 2 * sizeof(uint32_t) + getfname().size() + str.size();
    }
};

class msgtest4 : public message{
public:
    msgtest4(std::string s) : message(s) {}
    virtual serialize &serialization(serialize &s){
        s << getfname();
        return s;
    }
    virtual size_t serializesize(){
        return sizeof(uint32_t) + getfname().size();
    }
};

class msgtest5 : public message{
private:
    valtype tmp;

public:
    msgtest5(std::string s, valtype tmp) : message(s), tmp(tmp) {}
    virtual serialize &serialization(serialize &s){
        s << getfname() << tmp;
        return s;
    }
    virtual size_t serializesize(){
        return sizeof(uint32_t) + getfname().size() + tmp.serializesize();
    }
};

class msgtest6 : public message{
private:
    double a;
public:
    msgtest6(std::string s, double a) : message(s), a(a) {}
    virtual serialize &serialization(serialize &s){
        s << getfname() << a;
        return s;
    }
    virtual size_t serializesize(){
        return sizeof(uint32_t) + getfname().size() + sizeof(a);
    }
};

#endif
```
