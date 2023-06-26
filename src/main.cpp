/*
 * @Author: zgy
 * @Date: 2023-06-12 10:26:35
 * @LastEditTime: 2023-06-12 16:28:20
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /press_test/src/main.cpp
 */
#include <assert.h>

#include <gflags/gflags.h>

#include "common/log.h"
#include "common/conf.h"
#include "method/parse.h"
#include "method/request.h"
#include "method/mult_thread_deal.h"

// 设置命令行参数
DEFINE_uint32(port1, 8600, "mcs-in base port");
DEFINE_uint32(port2, 8601, "mcs-in test port");
DEFINE_string(dic_path, "", "path of mcs-in press test dictionary");
DEFINE_uint32(time_, 1, "time of press test: min");
DEFINE_double(qps, 1.0, "qps");
// 开启日志
int g_close_log = 0;

int main(int argc, char **argv){
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    // 获取日志配置   
    presstest::Conf conf_log("../conf/log.conf");
    // 初始化日志
    presstest::Log *log = presstest::Log::get_instance();
    assert(log->init(conf_log) == 0);
    presstest::MultDeal* mult_deal = presstest::MultDeal::get_instance();
    mult_deal->init();
    mult_deal->mult_deal_v2();
    return 0;
}