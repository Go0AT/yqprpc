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