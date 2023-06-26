/*
@Time    : 2023-06-13 17:22
@Author  : zhangguangyuan
@File    : parse.h
*/
#pragma once

#include<map>
#include<fstream>
#include<memory>

#include <gflags/gflags.h>

#include "common/log.h"
#include "rapidjson/document.h"
#include "common/block_queue.h"

extern int g_close_log;
DECLARE_string(dic_path);

namespace presstest{
class ParseDict{
public:
    static ParseDict* get_instance(){
        static ParseDict instance;
        return &instance;
    }
    int init(Conf &conf);
    // 可以读取字典中从第line_num行到之后_max_dict_size个行的内容，存储在_dict中
    int load_dict(int line_num);
    int clear_dict();

    std::map<int, rapidjson::Document*>* get_map();
    BlockDeque<rapidjson::Document*>* get_block();
    int get_block_size();
    // 解析json
    size_t parse_dict();
    // 多线程环境下，用一个生产者解析json
    int mult_parse_dict();
    // 由多个消费者线程调用
    rapidjson::Document* take_doc();

private:
    ParseDict() = default;
    ~ParseDict(){
        _ifs.close();
        auto it = _request_map.begin();
        for(;it != _request_map.end(); ++it){
            delete(it->second);
            it->second = NULL;
        }
        delete _request_deque;
    };
    const char* _dict_path;
    std::ifstream _ifs;
    std::map<int, std::string> _dict;
    std::map<int, rapidjson::Document*> _request_map; // 单进程解析后对象存入_request_map
    BlockDeque<rapidjson::Document*>* _request_deque;  // 多线程解析后存入阻塞队列
    int _max_dict_size; // dict的长度
    int _max_deque_size; // 阻塞队列最大长度
    int _pos;
    int _mode;
};
}

