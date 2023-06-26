#include <common/timer.h>

namespace presstest{
    void Timer::start(uint msec, bool immediate_run){
        if(msec <= 0){
            LOG_ERROR("定时任务时间间隔设置错误")
            return;
        }
        _msec = msec;
        _flag = false;
        _timer_thread = std::thread(std::bind(&presstest::Timer::run, this));
        _timer_thread.detach();
        _immediate_run = immediate_run;
    }
    void Timer::stop(){
        _cond.notify_all();
        _flag = true;
    }
    bool Timer::get_flag(){
        return _flag;
    }
    void Timer::run(){
        if(_immediate_run){
            _func();
        }
        while(!_flag){
            {
                std::unique_lock<std::mutex> locker(_mutex);
                // 等待设定的时间间隔，时间不到wait_for将阻塞
                std::function<bool(void)> temp = std::bind(&presstest::Timer::get_flag, this);
                _cond.wait_for(locker, std::chrono::milliseconds(_msec), temp);
            }
            _func();
        }
    }
}