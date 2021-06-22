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