#ifndef THREADPOOL_H__
#define THREADPOOL_H__

#include <set>
#include <atomic>
#include <queue>
#include <vector>
#include <thread>
#include <future>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <memory>

class threadpool{
private:
    const int MAXT;
    const int MINT;
    std::vector<std::thread> vec;
    std::queue<std::function<void()>> fque;
    std::set<int> posst;
    std::mutex stlock;
    std::mutex quelock;
    std::condition_variable cond;
    std::atomic<bool> running;
    std::atomic<int> idlthread;
    void poolcontrol();
    void addthread(int num);
public:
    threadpool(int lb,int ub);
    ~threadpool();
    template<typename F,typename ...Args>
    auto commit(F &&fun,Args &&...args) -> std::future<decltype(fun(args...))>;
    void test();
    void close();
};

template <typename F,typename ...Args>
auto threadpool::commit(F &&fun,Args &&...args) -> std::future<decltype(fun(args...))>{
    using restype = decltype(fun(args...));
    if(!running) return std::future<restype>();
    auto task = std::make_shared<std::packaged_task<restype()>>(std::bind(std::forward<F>(fun),std::forward<Args>(args)...));
    std::future<restype> ret = task->get_future();
    {
        std::lock_guard<std::mutex> lock(quelock);
        fque.emplace([task]()->void{
            (*task)();
        });
    }
    stlock.lock();
    // printf("idlthread = %d\n",(int)idlthread);
    if(!idlthread && !posst.empty()){
        printf("add\n");
        addthread(1);
    }
    stlock.unlock();
    cond.notify_one();
    return ret;
}

#endif