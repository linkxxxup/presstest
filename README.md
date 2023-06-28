presstest
==========

### 环境依赖
- 编译器：gcc 4.4.6
- 命令行解析库： gflags
- cpp网络客户端：libcurl

### 启动
- 在build.sh中修改发压的端口(分别向base环境和test环境发压)和压测词典路径，设置时间和qps
- 执行sh build.sh

### 文件说明
```bash
.
├── gflags   // 第三方库：解析命令行文件
├── build.sh  // 自动编译并运行的shell文件
├── CMakeLists.txt 
├── conf  // 配置文件
│   ├── log.conf //目录配置
│   ├── parse.conf //解析配置
│   └── request.conf //请求配置。比较重要的是需要配置ip，即请求发送的地址
├── lib 
├── log // 日志文件，建议每次执行压测任务都删一下日志
├── response_data // 存放响应结果， 响应内容和响应头单独存放。建议每次执行压测任务都删除下该文件夹。后续也可以通过该目录下的文件实现相关响应内容的分析
├── include
│   ├── common //公共工具
│   │   ├── block_queue.h //阻塞队列
│   │   ├── conf.h // 配置
│   │   ├── log.h //日志
│   │   ├── thread_pool.h //线程池，代码里设定为5
│   │   └── timer.h //定时器，按时执行任务
│   ├── method 
│   │   ├── mult_thread_deal.h // 多线程任务处理
│   │   ├── parse.h // 解析压测字典，可以在压测字典请求数量不够的情况下循环读入压测字典。
│   │   ├── request.h // 发送请求。可以对该文件进行改动并指定压测字典目录，实现批量请求发送
│   │   └── request_v2.h // 发送请求。可以对该文件进行改动并指定压测字典目录，实现批量请求发送。request.h有缺陷，经测试qps设定较大时，没法按照设定值发送。v2版本迭代了代码，做了改进
│   ├── property_tree // 第三方库：解析配置文件
│   └── rapidjson //第三方库：解析json
└── src 
    ├── common 
    │   ├── conf.cpp 
    │   ├── log.cpp 
    │   └── timer.cpp
    ├── main.cpp
    └── method
        ├── mult_thread_deal.cpp
        ├── parse.cpp
        ├── request.cpp
        └── request_v2.cpp
```

### 代码的主要执行逻辑是：
- v1版本：在mult_thread_deal.h中，生产者线程调用ParseDict::mult_parse_dict进行压测字典的读取，读取的Document对象放入阻塞队列中（当阻塞队列满时就阻塞，不再读。等待消费线程消费。）消费者线程与一个定时器绑定，定时器时间到了就从阻塞队列中取出一个任务并由线程池中的一个线程调用Request::multi_build_and_send进行处理。
- v2版本（6.19更新）: 通过curl_multi_perform使用一个线程异步发送请求（单线程但是使用io复用(select)），见request_v2.h。生产者线程还是使用v1版本，但消费者线程为RequestV2::mult_build_and_send_v2， 与一个定时器绑定，到定时时间后将easy_curl注册到multi_hand中。

### UML类图

### 特点：
- 通过定时器实现精准的qps，1qps即每秒发送一个请求。可以通过日志的时间戳查看请求是否以设定的qps来发送。
- 没有压测词典中请求体的个数限制，一个请求体也可以以相应的qps循环发送。
- 可拓展，可以作为批量发送请求的脚本，并能从response_data目录的文件下批量对相关响应内容做后续处理，提高功能测试和diff测试的速度。

### 写在后面
- 因为所用服务器的编译器版本太低，很多地方能使用lambda优化代码结构的部分但是还是写了完整函数。
- 服务器的curl版本也比较低，只能使用select实现io复用，而不能用libcurl新版本的curl_multi_poll。

