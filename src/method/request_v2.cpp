#include "method/request_v2.h"

namespace presstest{
int RequestV2::init(ParseDict* ins, Conf &conf){
    _pd = ins;
    _request_deque_ptr = ins->get_block();
    _success_request = 0;
    _fail_request = 0;
    _multi_hand = curl_multi_init();
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

void RequestV2::multi_build_and_send_v2(){
    auto doc_ptr = _pd->take_doc();
    add_curl_to_multi_hand(_port_test, doc_ptr);
    add_curl_to_multi_hand(_port_base, doc_ptr);
    delete doc_ptr;
}

void RequestV2::run(){
    int running_num = 0;
    int max_fd = 0;
    long time = 0;
    fd_set read_fd, write_fd, except_fd;
    struct timeval T;
    while(true){
        // 执行请求，返回正在执行的请求数量
        curl_multi_perform(_multi_hand, &running_num);
        // 等待有任务完成通知，有结果时立即返回，将完成的请求数量存放在ret_num中
        if(running_num){
            FD_ZERO(&read_fd);
            FD_ZERO(&write_fd);
            FD_ZERO(&except_fd);
            
            if(curl_multi_fdset(_multi_hand, &read_fd, &write_fd, &except_fd, &max_fd)){
                LOG_ERROR("curl_multi_fdset fail")
                return;
            }
            if(curl_multi_timeout(_multi_hand, &time)){
                LOG_ERROR("curl_multi_timeout");
                return;
            }
            if(max_fd == -1){
                //pass
            }else{
                if(select(max_fd + 1, &read_fd, &write_fd, &except_fd, NULL) < 0){
                    LOG_ERROR("select error");
                    return;
                }
            }
        }else{
            //pass
        }

        CURLMsg *msg = NULL;
        int msg_left = -1;
        while((msg = curl_multi_info_read(_multi_hand, &msg_left))){
            if(msg->msg == CURLMSG_DONE){
                char *code = NULL;
                CURL *curl = msg->easy_handle;
                curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &code);
                LOG_INFO("No.%d request code is %d", _success_request++, code);
                // 向文件中每一项响应内容后面加入换行符
                fputs("\n", _test_fp_res_content);
                curl_multi_remove_handle(_multi_hand, curl); //移除multi_handle中注册的easy curl
                put_curl(curl); // 将用完的curl放回到curl池中
            }else{
                //pass
            }
        }
    }
}
CURL* RequestV2::create_curl(){
    if(_curl_num > CURLNUM){
        return NULL;
    }
    CURL* curl = curl_easy_init();
    if(curl){
        // pass
    }else{
        LOG_ERROR("curl not init")
        abort();
    }
    // // 支持重定向
    // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    // // 保持会话
    // curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);
    // // 设置共享dnscache功能
    // curl_easy_setopt(curl, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    // 不验证主机名称
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    // 如果不设置该选项，会导致一些curl发送信号，结束mult_curl的wait。
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    // 设置连接超时时间
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 100);
    // 设置写入文件
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, _test_fp_res_content); //将返回的html主体数据输出到fp指向的文件
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, _test_fp_res_header); //将返回的http头输出到fp指向的文件
    return curl;
}

CURL* RequestV2::add_content_to_curl(int port, CURL * curl, rapidjson::Document *doc_ptr){
    if(doc_ptr->HasMember("method")){
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, (*doc_ptr)["method"].GetString());
    }else{
        LOG_ERROR("NO.%d http request not have method", _success_request)
        return NULL;
    }
    if(doc_ptr->HasMember("uri")){
        const std::string uri = "http://" + _ip + ":" + std::to_string((long long)port) + (*doc_ptr)["uri"].GetString();
        curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
    }else{
        LOG_ERROR("NO.%d http request not have uri", _success_request)
        return NULL;
    }
    struct curl_slist *headers = NULL;
    // header可能不是一个字符串，可能还是json，如是json则不能用GetString()
    // if(doc_ptr->HasMember("header")){
    //     headers = curl_slist_append(headers, (*doc_ptr)["header"].GetString());
    // }
    if(doc_ptr->HasMember("content")){
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (*doc_ptr)["content"].GetString());
    }
    return curl;
}
CURL* RequestV2::get_curl(){
    CURL* curl = NULL;
    _mutex_curl.lock();
    if(_list_curl.size() > 0){
        curl = _list_curl.front();
        _list_curl.pop_front();
    }
    _mutex_curl.unlock();
    if(curl == NULL){
        curl = create_curl();
    }
    return curl;
}
void RequestV2::put_curl(CURL* curl){
    _mutex_curl.lock();
    _list_curl.push_back(curl);
    _mutex_curl.unlock();
}
void RequestV2::add_curl_to_multi_hand(int port, rapidjson::Document *doc_ptr){
    CURL* curl = get_curl();
    if(curl != NULL){
        curl = add_content_to_curl(port, curl, doc_ptr);
    }else{
        LOG_ERROR("get curl fail")
        return;
    }
    CURLMcode code = curl_multi_add_handle(_multi_hand, curl);
    if(code != CURLM_OK){
        LOG_ERROR("No.%d add handle fail: %s", _fail_request++, curl_multi_strerror(code));
        put_curl(curl);
        return;
    }
}
}