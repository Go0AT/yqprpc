#ifndef BYTEORDER_H__
#define BYTEORDER_H__

#include <arpa/inet.h>
namespace byteorder{
    uint64_t htonll(uint64_t val);
    uint64_t ntohll(uint64_t val);
    template<typename T>
    T hotone(T val){
        switch(sizeof(val)){
            case 1:
                return val;
            case 2:
                return htons(val);
            case 4:
                return htonl(val);
            break;
        }
        if(__BYTE_ORDER == __BIG_ENDIAN)
            return val;
        uint64_t ret = htonll(*((uint64_t*)&val));
        return *((T*)&ret);
    }

    template<typename T>
    T netoho(T val){
        switch(sizeof(val)){
            case 1:
                return val;
            case 2:
                return ntohs(val);
            case 4:
                return htonl(val);
            break;
        }
        if(__BYTE_ORDER == __BIG_ENDIAN)
            return val;
        uint64_t ret = ntohll(*((uint64_t*)&val));
        return *((T*)&ret);
    }
};
#endif