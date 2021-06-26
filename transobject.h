#ifndef TRANSOBJECT_H__
#define TRANSOBJECT_H__

#include "serialize.h"
class valtype{
public:
    int a, b;
    serialize &serialization(serialize &s){
        s << a << b;
        return s;
    }
    serialize &deserialization(serialize &s){
        s >> a >> b;
        return s;
    }
    size_t serializesize(){
        return sizeof(a) + sizeof(b);
    }
};

#endif