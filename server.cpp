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