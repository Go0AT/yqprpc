#ifndef SERIALIZE_H__
#define SERIALIZE_H__

#include <vector>
#include <cstring>
#include <string>
#include "byteorder.h"
class streambuffer : public std::vector<char>{
private:
    size_t curpos;
public:
    streambuffer();
    streambuffer(const char *in,size_t lens);
    ~streambuffer() = default;
    bool iseof();
    const char *data();
    const char *currentdata();
    void reset();
    void offset(int k);
    void input(const char *in,size_t lens);

};

class serialize;

template <typename T>
struct class_serialize{
    template <typename U, serialize &(U::*)(serialize &) = &U::serialization>
    static constexpr bool check(U *){return true;}
    static constexpr bool check(...){return false;}
    static constexpr bool ret = check(static_cast<T *>(0));
};

template <typename T>
struct class_deserialize{
    template <typename U, serialize &(U::*)(serialize &) = &U::deserialization>
    static constexpr bool check(U *){return true;}
    static constexpr bool check(...){return false;}
    static constexpr bool ret = check(static_cast<T *>(0));
};

class serialize{
private:
    streambuffer iodevice;
public:
    serialize() = default;
    serialize(streambuffer sb);
    ~serialize() = default;
    void reset();
    size_t size();
    void write_raw_data(const char *in,size_t lens);
    const char * getdata();
    const char * getcurrentdata();
    void clear();
    template <typename T,typename std::enable_if<class_deserialize<T>::ret,T>::type * = nullptr>
    void output(T &t);
    template <typename T,typename std::enable_if<!class_deserialize<T>::ret,T>::type * = nullptr>
    void output(T &t);
    template <typename T,typename std::enable_if<class_serialize<T>::ret,T>::type * = nullptr>
    void input(T &t);
    template <typename T,typename std::enable_if<!class_serialize<T>::ret,T>::type * = nullptr>
    void input(T &t);
    template <typename T>
    serialize &operator >>(T &t);
    template <typename T>
    serialize &operator <<(T &t);
};


template <typename T>
serialize &serialize::operator >> (T &t){
    output(t);
    return *this;
}

template <typename T>
serialize &serialize::operator << (T &t){
    input(t);
    return *this;
}

template <typename T,typename std::enable_if<class_deserialize<T>::ret,T>::type *>
void serialize::output(T &t){
    t.deserialization(*this);
}

template <typename T,typename std::enable_if<!class_deserialize<T>::ret,T>::type *>
void serialize::output(T &t){
    t = byteorder::netoho(*((T *)(iodevice.currentdata())));
    iodevice.offset(sizeof(T));
}

template<>
void serialize::output(std::string &s);

template<typename T,typename std::enable_if<class_serialize<T>::ret,T>::type *>
void serialize::input(T &t){
    t.serialization(*this);
}

template <typename T, typename std::enable_if<!class_serialize<T>::ret, T>::type *>
void serialize::input(T &t){
    T val = byteorder::hotone(t);
    iodevice.input((const char *)&val, sizeof(T));
}

template<>
void serialize::input(std::string &s);

#endif