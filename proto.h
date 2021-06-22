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