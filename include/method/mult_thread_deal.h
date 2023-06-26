/*
@Time    : 2023-06-13 17:22
@Author  : zhangguangyuan
@File    : mult_thread_deal.h
*/
#pragma once

#include <map>
#include <mutex>
#include <thread>
#include <memory>

#include "common/thread_pool.h"
#include "common/block_queue.h"
#include "common/timer.h"
#include "rapidjson/document.h"
#include "method/parse.h"
#include "method/request.h"
#include "method/request_v2.h"

DECLARE_uint32(time_);
DECLARE_double(qps);

// TODO:可以修改lambda2，不把任务放入线程池处理，而是用curl_multi_add_handle将easy hand加入到multi hand中
namespace presstest{
class MultDeal{
public:
    static MultDeal* get_instance(){
        static MultDeal instance;
        return &instance;
    }
    void init();
    
    void single_deal(); // 版本一：后面写了mult_deal()后就没有维护该部分代码了，单进程主要用于debug
    void mult_deal(); // 版本二：多线程处理请求，qps过大时报告显示qps没达到设定值。
    void mult_deal_v2(); // 版本三：单线程但是用io复用，最终的报告显示能使用该方法能实现设置的qps的发送。
    
    void lambda1(); // mult_deal()调用的消费者线程
    void lambda2(); // 版本二和三的生产者线程
    void lambda3(); // mult_deal_v2()调用的消费者线程

private:
    MultDeal() = default;
    ~MultDeal(){
        delete _thread_pool;
        delete _timer;
    }

    presstest::ParseDict *_pd;
    presstest::Request* _rq;
    presstest::RequestV2* _rq_v2;
    std::function<int(void)> _put; // 生产者线程（唯一）调用
    std::function<void(void)> _take; // mult_deal()调用的消费者线程
    std::function<void(void)> _take_v2; // mult_deal_v2()调用的消费者线程
    presstest::ThreadPool *_thread_pool;
    presstest::Timer* _timer; //mult_deal()调用的定时器
    presstest::Timer* _timer2; //multdeal_v2()调用的定时器
    std::mutex _mutex;
    int _time;
    int _close;
    double _qps;
};
}
