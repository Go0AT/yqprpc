#include "yqprpc.h"
#include "funparam.h"
#include <vector>
#include <cstdio>
#include <iostream>
void test(){
    yqprpc client;
    client.asclient("127.0.0.1", 2021);
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
}

int main(){
    std::vector<std::thread> vec(500);
    for(int i = 0 ; i < 500 ; i++ )
        vec[i] = std::thread(test);
    for(int i = 0 ; i < 500 ; i++ )
        vec[i].join();
    // for(int i = 0 ; i < 9 ; i++ )
    //     fork();
    // test();
    // printf("o\n");
    return 0;
}