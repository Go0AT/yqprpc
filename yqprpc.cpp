
#include "yqprpc.h"
#include <cstdio>

yqprpc::yqprpc(){
    signal(SIGPIPE,SIG_IGN);
}

void yqprpc::asclient(std::string serip, int port){
    this->serip = serip;
    this->sport = port;
}

void yqprpc::asserver(int port){
    this->sport = port;
}

serialize *yqprpc::call_(std::string fname, const char *data, size_t lens){
    serialize *s = new serialize;
    if(!mp.count(fname)){
        printf("no bind!\n");
        return s;
    }
    auto fun = mp[fname];
    fun(s,data,lens);
    return s;
}

void yqprpc::servstart(){
    serv_exit = std::promise<void>();
    promisefuture = serv_exit.get_future();
    serv = std::thread(&yqprpc::run,this);
}

void yqprpc::servclose(){
    serv_exit.set_value();
    serv.join();
}

void yqprpc::run(){
    printf("start to run\n");
    network.mlisten(sport);
    threadpool pool(4,16);
    char buf[1024];
    std::set<edata*> st;
    int epfd = epoll_create(10000);
    epoll_event ev;
    ev.events = EPOLLIN;
    edata *pl = new edata;
    int listensd = network.getlistensd();
    pl->fd = listensd;
    ev.data.ptr = pl;
    epoll_ctl(epfd,EPOLL_CTL_ADD,listensd,&ev);
    std::atomic<int> cntr,cntc;
    cntc = 0;
    cntr = 0;
    while(true){
        if(promisefuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
            break;
        int num = epoll_wait(epfd,events,10000,1000);
        for(int i = 0 ; i < num ; i++ ){
            edata *ptr = (edata*)events[i].data.ptr;
            if(ptr->fd == listensd){
                int connsd = accept(listensd,NULL,NULL);
                if(connsd < 0){
                    perror("connect()");
                    continue;
                }
                ++cntc;
                ipv4tcp::setnoblock(connsd);
                ev.events = (EPOLLIN | EPOLLET);
                ptr = new edata;
                st.insert(ptr);
                ptr->fd = connsd;
                ev.data.ptr = ptr;
                epoll_ctl(epfd,EPOLL_CTL_ADD,connsd,&ev);
                continue;
            }
            if(events[i].events & EPOLLIN){
                while(true){
                    ssize_t lens = read(ptr->fd,buf,1024);
                    if(lens < 0 ){
                        if(errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                        perror("read()");
                        epoll_ctl(epfd,EPOLL_CTL_DEL,ptr->fd,NULL);
                        close(ptr->fd);
                        st.erase(ptr);
                        delete ptr;
                        // printf("in1?\n");
                        break;
                    }else if(lens > 0){
                        ptr->s.write_raw_data(buf,lens);
                    }else{
                        if(ptr->s.size() < sizeof(uint32_t)){
                            epoll_ctl(epfd,EPOLL_CTL_DEL,ptr->fd,NULL);
                            close(ptr->fd);
                            st.erase(ptr);
                            delete ptr;
                            // printf("in2?\n");
                            break;
                        }
                        uint32_t recvlens;
                        ptr->s >> recvlens;
                        if(recvlens != ptr->s.size()){
                            epoll_ctl(epfd, EPOLL_CTL_DEL, ptr->fd, NULL);
                            // printf("in3?\n");
                            close(ptr->fd);
                            st.erase(ptr);
                            delete ptr;
                            break;
                        }
                        std::string fname;
                        ptr->s >> fname;
                        epoll_ctl(epfd,EPOLL_CTL_DEL,ptr->fd,NULL);
                        pool.commit([epfd,this,ptr,fname]()->void{
                            serialize *s;
                            s = call_(fname,ptr->s.getcurrentdata(),ptr->s.size() - fname.size() - sizeof(uint32_t));
                            ptr->s = *s;
                            delete s;
                            epoll_event ev;
                            ev.events = (EPOLLOUT | EPOLLET);
                            ev.data.ptr = ptr;
                            epoll_ctl(epfd,EPOLL_CTL_ADD,ptr->fd,&ev);
                            return ;
                        });
                        break;
                    }
                }
            }
            if(events[i].events & EPOLLOUT){
                size_t pos = 0;
                bool legfalse = false;
                while(true){
                    ssize_t lens = write(ptr->fd,ptr->s.getdata() + pos,ptr->s.size() - pos);
                    if(lens < 0){
                        if(errno == EAGAIN || errno == EWOULDBLOCK){
                            perror("write()");
                            legfalse = true;
                        }
                        else perror("write()");
                        break;
                    }
                    pos += lens;
                    if(pos == ptr->s.size()) break;
                }
                if(legfalse) continue;
                epoll_ctl(epfd,EPOLL_CTL_DEL,ptr->fd,NULL);
                close(ptr->fd);
                st.erase(ptr);
                delete ptr;
            }
        }
    }
    pool.close();
    for(auto & ptr : st){
        close(ptr->fd);
        delete ptr;
    }
    st.clear();
    delete pl;
    close(network.getlistensd());
    close(epfd);
    return ;
}


//////
template<>
uint32_t getserializesize(std::string &s){
    return sizeof(uint32_t) + s.size();
}