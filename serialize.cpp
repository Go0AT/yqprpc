#include "serialize.h"

streambuffer::streambuffer(){
    curpos = 0;
}

streambuffer::streambuffer(const char *in,size_t lens){
    curpos = 0;
    for(int i = 0 ; i < lens ; i++ )
        push_back(in[i]);
}


void streambuffer::reset(){
    curpos = 0;
}

const char *streambuffer::data(){
    return &((*this)[0]);
}

const char *streambuffer::currentdata(){
    return &((*this)[curpos]);
}

void streambuffer::offset(int k){
    curpos += k;
}

bool streambuffer::iseof(){
    return curpos >= size();
}

void streambuffer::input(const char *in,size_t lens){
    for(int i = 0 ; i < lens ; i++ )
        push_back(in[i]);
}

//**********************

serialize::serialize(streambuffer sb){
    iodevice = sb;
}

void serialize::reset(){
    iodevice.reset();
}

size_t serialize::size(){
    return iodevice.size();
}

void serialize::clear(){
    iodevice.reset();
    iodevice.clear();
}

void serialize::write_raw_data(const char *in,size_t lens){
    iodevice.input(in,lens);
}

const char *serialize::getdata(){
    return iodevice.data();
}

const char *serialize::getcurrentdata(){
    return iodevice.currentdata();
}

template <>
void serialize::input(std::string &s){
    uint32_t lens = s.size();
    lens = byteorder::hotone(lens);
    iodevice.input((const char *)&lens, sizeof(lens));
    iodevice.input(s.c_str(), s.size());
}

template <>
void serialize::output(std::string &s){
    uint32_t lens = byteorder::netoho(*((uint32_t *)(iodevice.currentdata())));
    iodevice.offset(sizeof(lens));
    for (int i = 0; i < lens; i++)
        s.push_back(*(iodevice.currentdata() + i));
    iodevice.offset(lens);
}