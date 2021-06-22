# yqprpc
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
