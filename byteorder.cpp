#include "byteorder.h"

uint64_t byteorder::htonll(uint64_t val){
    if (__BYTE_ORDER == __BIG_ENDIAN)
        return val;
    return (((uint64_t)htonl(val) << 32) | htonl(val >> 32));
}

uint64_t byteorder::ntohll(uint64_t val){
    if (__BYTE_ORDER == __BIG_ENDIAN)
        return val;
    return (((uint64_t)ntohl(val) << 32) | ntohl(val >> 32));
}