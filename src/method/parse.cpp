#include "method/parse.h"

namespace presstest{
int ParseDict::init(Conf &conf){
    _max_dict_size = stoi(conf.get("parse.max_dict_size"));
    _max_deque_size = stoi(conf.get("parse.max_deque_size"));
    _mode = stoi(conf.get("parse.mode"));
    _dict_path = FLAGS_dic_path.c_str();
    _ifs.open(_dict_path, std::ios::in);
    _pos = 0;
    _request_deque = new BlockDeque<rapidjson::Document*>(_max_deque_size);
    if(_ifs.is_open()){
        LOG_INFO("dict file is open, please check the path: %s", _dict_path)
        return 0;
    }else{
        LOG_ERROR("dict file not open")
        return 1;
    }
}
int ParseDict::load_dict(int line_num){
    clear_dict();
    int n = 0;
    std::string line;
    // 如果line_num超过文件实际行数，也会退出, _dict将为空
    while(getline(_ifs, line)){
        if(n >= line_num){
            _dict[n++] = line;
        }else{
            ++n;
        }
        if(n - line_num + 1 > _max_dict_size){
            break;  // 达到规定的最大值结束，TODO: 文件读到末尾后再从第一行开始读()
        }
    }
    // 返回已经读取过的行数
    return _dict.size();
}
int ParseDict::clear_dict(){
    _dict.clear();
}
std::map<int, rapidjson::Document*>* ParseDict::get_map(){
    return &_request_map;
}
BlockDeque<rapidjson::Document*>* ParseDict::get_block(){
    return _request_deque;
}
int ParseDict::get_block_size(){
    return _request_deque->size();
}
size_t ParseDict::parse_dict(){
    auto it = _dict.begin();
    for(; it != _dict.end(); ++it){
        auto doc_ptr = new rapidjson::Document();
        doc_ptr->Parse(it->second.c_str());
        _request_map[it->first] = doc_ptr;
    }
    return _request_map.size();
}
int ParseDict::mult_parse_dict(){
    auto doc_ptr = new rapidjson::Document();
    if(_dict.find(_pos) != _dict.end()){
        doc_ptr->Parse(_dict[_pos++].c_str());
        _request_deque->push_back(doc_ptr);
    }else{
        int n = load_dict(_pos);
        if(n == 0){ // 表示已经到达文件末尾，没有新数据读入，需要重新开始读
            _ifs.clear();
            _ifs.seekg(0, std::ios::beg);
            _pos = 0;
            load_dict(0);
        }
    }
    return 0;
}
rapidjson::Document* ParseDict::take_doc(){
    rapidjson::Document *doc_ptr;
    _request_deque->pop(doc_ptr);
    return doc_ptr; // 记得释放内存
}
}