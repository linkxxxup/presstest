/*
@Time    : 2023-06-16 10:31
@Author  : zhangguangyuan
@File    : request.h
*/

#pragma once
#include <mutex>
#include <list>

#include <curl/curl.h>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "method/parse.h"
#include "common/conf.h"

DECLARE_uint32(port1);
DECLARE_uint32(port2);

namespace presstest{
class RequestV2{
    static const int CURLNUM = 100;

public:
    static RequestV2* get_instance(){
        static RequestV2 instance;
        return &instance;
    }
    int init(ParseDict* ins, Conf &conf);
    // 供mult_thread_deal.h调用，向mult_hand中注册easy_curl
    void multi_build_and_send_v2();
    // 供mult_thread_deal.h调用，开启mult_hand监听
    void run();
    
private:
    // 创建curl
    CURL* create_curl();
    // 向curl中装填请求内容
    CURL* add_content_to_curl(int port, CURL * curl, rapidjson::Document *doc_ptr);
    // 从curl池中取出一个curl，如果没有则创建一个curl
    CURL* get_curl();
    // 将curl重新放回
    void put_curl(CURL* curl);
    // 将一个curl实例注册到multi_handle中
    void add_curl_to_multi_hand(int port, rapidjson::Document *doc_ptr);

    RequestV2() = default;
    ~RequestV2(){
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
        if(_multi_hand != NULL){
            curl_multi_cleanup(_multi_hand);
            _multi_hand = NULL;
        }
        curl_global_cleanup();
    }

    ParseDict* _pd;
    BlockDeque<rapidjson::Document*>* _request_deque_ptr;
    CURL* _multi_hand;
    int _curl_num;
    FILE* _test_fp_res_header;
    FILE* _test_fp_res_content;
    FILE* _base_fp_res_header;
    FILE* _base_fp_res_content;
    int _port_test;
    int _port_base;
    std::string _ip;
    int _success_request;
    int _fail_request;
    std::mutex _mutex_curl; 
    std::list<CURL*> _list_curl;
};
}