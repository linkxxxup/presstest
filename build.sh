#!/bin/bash
# @Author: zgy
# @Date: 2023-06-12 10:26:35
# @LastEditTime: 2023-06-13 14:48:37
# @LastEditors: Please set LastEditors
# @Description: In User Settings Edit
# @FilePath: /press_test/build.sh

function main(){
    set -e
    rm -rf `pwd`/build
    mkdir `pwd`/build
    mkdir -p `pwd`/log
    mkdir -p `pwd`/response_data
    cd `pwd`/build &&
            cmake .. &&
            make
    echo '--------output---------'
    cd ../bin
    # ./exepress.out --port1=$1 --port2=$2 --dic_path=$3 --time_=$4 --qps=$5
     ./exepress.out --port1=8122 --port2=8123 --dic_path=/home/cxkgg/Documents/presstest/mcs_in_press_dict --time_=5 --qps=1
}

main "$@"

