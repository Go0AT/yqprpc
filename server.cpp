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