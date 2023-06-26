#include "method/request.h"

namespace presstest{
int Request::init(ParseDict* ins, Conf &conf){
    _pd = ins;
    _request_map_ptr = ins->get_map();
    _request_deque_ptr = ins->get_block();
    _success_request = 0;
    _fail_request = 0;
    // 打开四个接收结果的的文件
    std::string test_header_path = "../response_data/" 
    + conf.get("request.test_dir_res_header") == ""? "test_res_header" : conf.get("request.test_dir_res_header");
    std::string test_content_path = "../response_data/" 
    + conf.get("request.test_dir_res_content") == ""? "test_res_content" : conf.get("request.test_dir_res_content");
    std::string base_header_path = "../response_data/" 
    + conf.get("request.base_dir_res_header") == ""? "base_res_header" : conf.get("request.base_dir_res_header");
    std::string base_content_path = "../response_data/" 
    + conf.get("request.base_dir_res_content") == ""? "base_res_content" : conf.get("request.base_dir_res_content");
    if((_test_fp_res_header = fopen(test_header_path.c_str(), "a")) == NULL){
        LOG_ERROR("OPEN /response_data/test_res_header error")
        return 1;
    }
    if((_test_fp_res_content = fopen(test_content_path.c_str(), "a")) == NULL){
        LOG_ERROR("OPEN /response_data/test_res_content error")
        return 1;
    }
    if((_base_fp_res_header = fopen(base_header_path.c_str(), "a")) == NULL){
        LOG_ERROR("OPEN /response_data/test_res_header error")
        return 1;
    }
    if((_base_fp_res_content = fopen(base_content_path.c_str(), "a")) == NULL){
        LOG_ERROR("OPEN /response_data/test_res_content error")
        return 1;
    }
    // 初始化端口和地址
    _port_base = FLAGS_port1;
    _port_test = FLAGS_port2;
    _ip = conf.get("request.ip");
    curl_global_init(CURL_GLOBAL_ALL);
    return 0;
}

int Request::build_and_send(){
    CURL* curl = curl_easy_init();
    // 初始化curl
    if(curl) {
        LOG_INFO("curl is init")
    }else{
        LOG_ERROR("curl not init")
        abort();
    }
    CURLcode res;
    int success_request = 0;
    for(size_t i = 0; i < _request_map_ptr->size(); ++i){
        auto doc_ptr = (*_request_map_ptr)[i];
        if(doc_ptr->HasMember("method")){
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, (*doc_ptr)["method"].GetString());
        }else{
            LOG_ERROR("NO.%d http request not have method", i)
            continue;
        }
        if(doc_ptr->HasMember("uri")){
            const std::string uri = "http://" + _ip + ":" + std::to_string((long long)_port_test) + (*doc_ptr)["uri"].GetString();
            curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
        }else{
            LOG_ERROR("NO.%d http request not have uri", i)
            continue;
        }
        struct curl_slist *headers = NULL;
        // header可能不是一个字符串，可能还是json
        // if(doc_ptr->HasMember("header")){
        //     headers = curl_slist_append(headers, (*doc_ptr)["header"].GetString());
        // }
        if(doc_ptr->HasMember("content")){
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (*doc_ptr)["content"].GetString());
        }
        // 发送请求
        res = curl_easy_perform(curl);
        // 释放请求头
        curl_slist_free_all(headers);
        // 请求失败
        if(res != 0){
            LOG_ERROR("NO.%d http request failed, pleace check res content file", i)
            continue;
        }
        ++success_request;
    }
    curl_easy_cleanup(curl);
    return success_request;
}
// 供mult_thread_deal多线程模式下进行调用，但是每个线程发送请求时curl_easy_perform会阻塞，经测试不能适用于qps比较大的场景
void Request::multi_build_and_send(){
    CURL* curl_base = curl_easy_init();
    CURL* curl_test = curl_easy_init();
    // 设置写入文件
    curl_easy_setopt(curl_test, CURLOPT_WRITEDATA, _test_fp_res_content); //将返回的html主体数据输出到fp指向的文件, 存储test
    curl_easy_setopt(curl_test, CURLOPT_HEADERDATA, _test_fp_res_header); //将返回的http头输出到fp指向的文件，存储test
    curl_easy_setopt(curl_base, CURLOPT_WRITEDATA, _base_fp_res_content); //将返回的html主体数据输出到fp指向的文件, 存储base
    curl_easy_setopt(curl_base, CURLOPT_HEADERDATA, _base_fp_res_header); //将返回的http头输出到fp指向的文件，存储base
    // 初始化curl
    if(curl_base && curl_test) {
        // LOG_INFO("curl is init")
    }else{
        LOG_ERROR("curl not init")
        abort();
    }
    CURLcode res_test;
    CURLcode res_base;
    auto doc_ptr = _pd->take_doc();
    if(doc_ptr->HasMember("method")){
        curl_easy_setopt(curl_base, CURLOPT_CUSTOMREQUEST, (*doc_ptr)["method"].GetString());
        curl_easy_setopt(curl_test, CURLOPT_CUSTOMREQUEST, (*doc_ptr)["method"].GetString());
    }else{
        LOG_ERROR("NO.%d http request not have method", _success_request)
        return;
    }
    if(doc_ptr->HasMember("uri")){
        const std::string uri_test = "http://" + _ip + ":" + std::to_string((long long)_port_test) + (*doc_ptr)["uri"].GetString();
        const std::string uri_base = "http://" + _ip + ":" + std::to_string((long long)_port_base) + (*doc_ptr)["uri"].GetString();
        curl_easy_setopt(curl_base, CURLOPT_URL, uri_base.c_str());
        curl_easy_setopt(curl_test, CURLOPT_URL, uri_test.c_str());
    }else{
        LOG_ERROR("NO.%d http request not have uri", _success_request)
        return;
    }
    struct curl_slist *headers = NULL;
    // header可能不是一个字符串，可能还是json，如是json则不能用GetString()
    // if(doc_ptr->HasMember("header")){
    //     headers = curl_slist_append(headers, (*doc_ptr)["header"].GetString());
    // }
    if(doc_ptr->HasMember("content")){
        curl_easy_setopt(curl_base, CURLOPT_POSTFIELDS, (*doc_ptr)["content"].GetString());
        curl_easy_setopt(curl_test, CURLOPT_POSTFIELDS, (*doc_ptr)["content"].GetString());
    }
    // 发送请求
    // 瓶颈在这里，curl_easy_perform是同步的，只有等该函数返回时才继续往后进行，程序的执行受到了服务器回调时间的影响
    res_base = curl_easy_perform(curl_base);
    res_test = curl_easy_perform(curl_test);
    // 释放请求头
    curl_slist_free_all(headers);
    // 清除curl对象
    curl_easy_cleanup(curl_base);
    curl_easy_cleanup(curl_test);
    // 请求失败, 注意：base环境失败了，test也不会执行
    if(res_base != 0){
        LOG_ERROR("base: NO.%d http request failed, pleace check res content file", _fail_request++)
        return;
    }
    if(res_test != 0){
        LOG_ERROR("test: NO.%d http request failed, pleace check res content file", _fail_request++)
        return;
    }
    _mutex.lock();
    ++_success_request; // 指两个环境都发送成功
    _mutex.unlock();
    // 向文件中每一项响应内容后面加入换行符
    fputs("\n", _test_fp_res_content);
    fputs("\n", _base_fp_res_content);
    delete doc_ptr;
    doc_ptr = NULL;
    LOG_INFO("send http request No.%d success", _success_request);
}
}
