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