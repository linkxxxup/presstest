/*
@Time    : 2023-06-13 17:22
@Author  : zhangguangyuan
@File    : request.h
*/
#pragma once
#include <mutex>

#include <curl/curl.h>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "method/parse.h"
#include "common/conf.h"

DECLARE_uint32(port1);
DECLARE_uint32(port2);

namespace presstest{
class Request{
public:
    static Request* get_instance(){
        static Request instance;
        return &instance;
    }
    int init(ParseDict* ins, Conf &conf);
    // 单线程调用，根据_request_map循环构造请求并发送。
    // 目前已经不用了，代码其他部分更新时，这块一直没有同步
    int build_and_send();
    // 多线程调用, 使用request_queue
    // 供mult_thread_deal多线程模式下进行调用，但是每个线程发送请求时curl_easy_perform会阻塞，经测试不能适用于qps比较大的场景
    void multi_build_and_send();
    
private:
    Request() = default;
    ~Request(){
        if(_test_fp_res_header){
            fclose(_test_fp_res_header);
        }
        if(_test_fp_res_content){
            fclose(_test_fp_res_content);
        }
        if(_base_fp_res_header){
            fclose(_base_fp_res_header);
        }
        if(_test_fp_res_content){
            fclose(_base_fp_res_content);
        }
        curl_global_cleanup();
    }

    ParseDict* _pd;
    std::map<int, rapidjson::Document*>* _request_map_ptr;
    BlockDeque<rapidjson::Document*>* _request_deque_ptr;
    FILE* _test_fp_res_header;
    FILE* _test_fp_res_content;
    FILE* _base_fp_res_header;
    FILE* _base_fp_res_content;
    int _port_test;
    int _port_base;
    std::string _ip;
    int _success_request;
    int _fail_request;
    std::mutex _mutex; 
};
}