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