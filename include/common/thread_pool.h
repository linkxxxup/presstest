/*
@Time    : 2023-06-13 17:22
@Author  : zhangguangyuan
@File    : thread_pool.h
*/
#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <assert.h>

namespace presstest{
class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 8): _pool(std::make_shared<Pool>()),  _thread_count(threadCount){
        assert(threadCount > 0);
    }
    void create_threads(){
        for(size_t i = 0; i < _thread_count; i++) {
            std::thread t1(&presstest::ThreadPool::lamda1, this);
            t1.detach();
        }
    }
    // gcc4.4.6不支持lambda
    void lamda1(){
        std::unique_lock<std::mutex> locker(_pool->_mtx);
        while(true) {
            if(!_pool->_tasks.empty()) {
                auto task = std::move(_pool->_tasks.front());
                _pool->_tasks.pop();
                locker.unlock();
                task();
                locker.lock();
            }
            else if(_pool->_is_closed) break;
            else _pool->_cond.wait(locker);
        }
    }

    ThreadPool() = default;

    // ThreadPool(ThreadPool&&) = default;

    ~ThreadPool() {
        if(static_cast<bool>(_pool)) {
            {
                std::lock_guard<std::mutex> locker(_pool->_mtx);
                _pool->_is_closed = true;
            }
            _pool->_cond.notify_all();
        }
    }

    template<class F>
    void add_task(F&& task) {
        {
            std::lock_guard<std::mutex> locker(_pool->_mtx);
            _pool->_tasks.emplace(std::forward<F>(task));
        }
        _pool->_cond.notify_one();
    }

private:
    struct Pool {
        std::mutex _mtx;
        std::condition_variable _cond;
        bool _is_closed;
        std::queue<std::function<void()>> _tasks;
    };
    std::shared_ptr<Pool> _pool;
    int _thread_count;
};
}


