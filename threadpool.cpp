#include "threadpool.h"
#include <cstdio>

threadpool::threadpool(int lb,int ub):MAXT(ub),MINT(lb),vec(MAXT){
    running = true;
    for(int i = 0 ; i < MAXT ; i++ )
        posst.insert(i);
    addthread(MINT);
    idlthread = MINT;
}

threadpool::~threadpool(){
    running = false;
    cond.notify_all();
    for(int i = 0 ; i < MAXT ; i++ ){
        if(posst.count(i)) continue;
        if(vec[i].joinable()) vec[i].join();
    }
    while(!fque.empty()) fque.pop();
}

void threadpool::close(){
    running = false;
    cond.notify_all();
    for (int i = 0; i < MAXT; i++){
        // if (!posst.count(i)) continue;
        if (vec[i].joinable()) vec[i].join();
    }
    while (!fque.empty()) fque.pop();
}


void threadpool::addthread(int num){
    while(!posst.empty() && num){
        --num;
        int dex = *posst.begin();
        if(vec[dex].joinable()) vec[dex].join();
        vec[dex] = std::thread([dex,this]()->void{
            while(running){
                std::function<void()> fun;
                {
                    std::lock(quelock,stlock);
                    std::unique_lock<std::mutex> lock(quelock,std::adopt_lock);
                    if(!running || (fque.empty() && MAXT  - posst.size() > MINT)){
                        --idlthread;
                        posst.insert(dex);
                        stlock.unlock();
                        return ;
                    }
                    stlock.unlock();
                    cond.wait(lock,[this]()->bool{
                        return !running || !fque.empty();
                    });
                    --idlthread;
                    if(!running){
                        stlock.lock();
                        posst.insert(dex);
                        stlock.unlock();
                        return ;
                    }
                    fun = std::move(fque.front());
                    fque.pop();
                }
                fun();
                ++idlthread;
            }
        });
        ++idlthread;
        posst.erase(dex);
    }
    return ;
}

void threadpool::test(){
    while(true){
        stlock.lock();
        for(int i = 0 ; i < MAXT ; i++ ){
            printf("%c","RS" [posst.count(i)]);
        }
        printf("\n");
        stlock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}