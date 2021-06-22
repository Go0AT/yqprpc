# yqprpc

可以定义自己想要当作参数传输的对象，把对象声明放在proto.h中，需要实现 serialize &serialization(serialize &s)和serialize &deserialization(serialize &s)方法，基本类型(除了__int128)以及std::string可以直接作为客户端调用call<>方法的参数。但是本框架最多只能绑定含五个参数的函数(调用类中的函数时类对象指针不作为五个参数中的一个)，而且绑定函数时，使用函数名作为键，所以若函数同名，则会取消先前绑定的同名函数。本框架假设在网络传输时不会出现错误，本框架使用C++ 11编译。

#### 下面是一个小例子

#### Proto

```C++
#ifndef PROTO_H__
#define PROTO_H__

#include "serialize.h"

class valtype{
public:
    int a,b;
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

#### Server

```C++
#include "yqprpc.h"
#include "proto.h"
#include <iostream>
#include <algorithm>
int test(int a,int b,int c){
    return a + b + c;
}

void test2(std::string str){
    std::cout << str << std::endl;
}

std::string test3(std::string str){
    std::reverse(str.begin(),str.end());
    return str;
}

class A{
public:
    int a;
    void test4(){
        std::cout << a << std::endl;
    }
};

void test5(valtype tmp){
    std::cout << tmp.a << std::endl;
    std::cout << tmp.b << std::endl;
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
    server.run();
    return 0;
}
```

#### Client

```C++
#include "yqprpc.h"
#include "proto.h"
#include <cstdio>
#include <iostream>

int main(){
    yqprpc client;
    client.asclient("127.0.0.1",2021);
    int ret = client.call<int>("test",1,2,3);
    printf("ret = %d\n",ret);
    std::string str("123");
    client.call<void>("test2",str);
    str = client.call<std::string>("test3",str);
    std::cout << str << std::endl;
    client.call<void>("test4");
    valtype tmp;
    tmp.a = 456;
    tmp.b = 654;
    client.call<void>("test5",tmp);
    return 0;
}
```
