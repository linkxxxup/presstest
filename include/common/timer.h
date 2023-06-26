/*
@Time    : 2023-06-14 10:49
@Author  : zhangguangyuan
@File    : timer.h
*/

#pragma once

#include <thread>
#include <chrono>
#include <condition_variable>

#include "common/log.h"

extern int g_close_log;
namespace presstest
{
class Timer{
public:
    template<typename F>
    explicit Timer(F &func): _func(func){}
    ~Timer(){}

    void start(uint msec, bool immediate_run = false);
    void stop();
    bool get_flag();

private:
    void run();
    std::function<void(void)> _func; //执行的定时器任务
    uint _msec; //间隔时间
    bool _immediate_run;
    bool _flag;
    std::thread _timer_thread;
    std::mutex _mutex;
    std::condition_variable _cond;
};
} 