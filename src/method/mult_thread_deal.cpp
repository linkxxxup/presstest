#include "method/mult_thread_deal.h"

namespace presstest{
void MultDeal::init(){
    _time = FLAGS_time_ * 60; // 时间换算成秒
    _qps = 1000 / FLAGS_qps; //qps为1~100，需要换算成以ms为单位的时间间隔
    _pd = presstest::ParseDict::get_instance();
    _rq = presstest::Request::get_instance();
    _rq_v2 = presstest::RequestV2::get_instance();
    _put = std::bind(&presstest::ParseDict::mult_parse_dict, _pd);
    _take = std::bind(&presstest::Request::multi_build_and_send, _rq);
    _take_v2 = std::bind(&presstest::RequestV2::multi_build_and_send_v2, _rq_v2);
    _close = 0;
    _thread_pool = new ThreadPool(5); //创建5个线程的线程池
    _thread_pool->create_threads();
    // 将消费者函数与定时器绑定
    std::function<void(void)> task = std::bind(&presstest::MultDeal::lambda1, this);
    _timer = new Timer(task);
    std::function<void(void)> task2 = std::bind(&presstest::MultDeal::lambda3, this);
    _timer2 = new Timer(task2);
}
void MultDeal::mult_deal(){
    // 初始化解析配置
    presstest::Conf conf_parse("../conf/parse.conf");
    // 进行文件解析
    CHECK_RET(_pd->init(conf_parse) == 0, "parse_dict init false")
    // 获取请求配置
    presstest::Conf conf_request("../conf/request.conf");
    // 构造请求并发送x
    CHECK_RET(_rq->init(_pd, conf_request) == 0, "init request error")
    std::time_t begin = std::time(NULL);
    // 生产者线程进行生产
    std::thread producer_thread(&MultDeal::lambda2, this);
    producer_thread.detach();
    // 消费者线程进行消费
    _timer->start(_qps, true); 
    // 执行直到时间超时
    while(!_close){
        std::time_t end = std::time(NULL);
        if(end - begin >= _time){
            _close = 1; //时间到
        }
    }
    _timer->stop();
}
// mult_deal调用
void MultDeal::lambda1(){
        // 放入线程池中处理
        _thread_pool->add_task(_take);
}
void MultDeal::lambda2(){
    while(!_close){
        int flag = _put();
        if(flag == 1){
            _close = 1;
        }
    }
}

// 该函数已经不用了，代码中其他部分有变化也没修改该函数
void MultDeal::single_deal(){
    // 初始化解析配置
    presstest::Conf conf_parse("../conf/parse.conf");
    // 进行文件解析
    CHECK_RET(_pd->init(conf_parse) == 0, "parse_dict init false")
    CHECK_RET(_pd->load_dict(0) > 0, "dict is empty")
    CHECK_RET(_pd->parse_dict() > 0, "not parse dict")
    // 获取请求配置
    presstest::Conf conf_request("../conf/request.conf");
    // 构造请求并发送
    CHECK_RET(_rq->init(_pd, conf_request) == 0, "init request error")
    int response_num = _rq->build_and_send();
    CHECK_RET(response_num > 0, "build and send request none")
    std::cout << "response num: " << response_num << std::endl; 
}

void MultDeal::mult_deal_v2(){
    // 初始化解析配置
    presstest::Conf conf_parse("../conf/parse.conf");
    // 进行文件解析
    CHECK_RET(_pd->init(conf_parse) == 0, "parse_dict init false")
    // 获取请求配置
    presstest::Conf conf_request("../conf/request.conf");
    // 构造请求并发送x
    CHECK_RET(_rq_v2->init(_pd, conf_request) == 0, "init request error")
    std::time_t begin = std::time(NULL);
    // 生产者线程进行生产
    std::thread producer_thread(&MultDeal::lambda2, this);
    producer_thread.detach();
    // 消费者线程进行消费
    // std::thread consumer_thread(&MultDeal::lambda3, this);
    // consumer_thread.detach();
    _timer2->start(_qps, true);
    // 执行直到时间超时
    _rq_v2->run(); 
    while(!_close){
        std::time_t end = std::time(NULL);
        if(end - begin >= _time){
            _close = 1; //时间到
        }
    }
    _timer2->stop();
}
// mult_deal_v2调用
void MultDeal::lambda3(){
    _take_v2();
}
}