# yqprpc

可以定义自己想要当作参数传输的对象，把对象声明放在transobject.h中，需要实现 serialize &serialization(serialize &s)和serialize &deserialization(serialize &s)和size_t serializesize()方法，客户端在使用call<>()时，需要传入调用远程函数需要的参数类(继承自message虚基类(放于proto.h中)，需要实现virtual serialize &serialization(serialize &s)方法以及virtual size_t serializesize()方法)，参数类存放于funparam.h中，使用参数类可以使服务器bind的可被远程调用的函数所需要的参数的数量不受限制(把需要调用的函数用新函数包装起来，相当于让用户解决了这个问题)(之前想过用C++ 11泛型解决，还没想到解决办法)。框架使用函数名作为键，所以若函数同名，则会取消先前绑定的同名函数。使用本框架时，可以通过函数返回值val_t类型的issuccess()方法查看远程调用是否成功，val_t中还有原本函数调用的返回值(唯一可能没有解决的问题是connect的时如果网不好，会等待75s才会报连接超时错误)，本框架使用C++ 11编译。

#### 下面是一个小例子

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
    server.run();
    return 0;
}
```

#### Client

```C++
#include "yqprpc.h"
#include "funparam.h"
#include <cstdio>
#include <iostream>

int main(){
    yqprpc client;
    client.asclient("127.0.0.1",2021);
    int ret = client.call<int>(msgtest("test",1,2,3));
    printf("ret = %d\n",ret);
    std::string str("123");
    client.call<void>(msgtest2("test2",str));
    str = client.call<std::string>(msgtest3("test3",str));
    std::cout << str << std::endl;
    client.call<void>(msgtest4("test4"));
    valtype tmp;
    tmp.a = 456;
    tmp.b = 654;
    client.call<void>(msgtest5("test5",tmp));
    std::cout << client.call<double>(msgtest6("test6",3.14)) << std::endl;
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
};

class msgtest4 : public message{
public:
    msgtest4(std::string s) : message(s) {}
    virtual serialize &serialization(serialize &s){
        s << getfname();
        return s;
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
};

#endif
```
