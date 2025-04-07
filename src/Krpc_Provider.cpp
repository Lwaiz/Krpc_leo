/**
  ******************************************************************************
  * @file           : Krpc_Provider.cpp
  * @author         : 18483
  * @brief          : KrpcProvider 核心实现
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */

#include "Krpc_Logger.h"
#include "Krpc_Application.h"
#include "Krpcheader.pb.h"
#include "Krpc_Provider.h"
#include <iostream>

/*
 *    service_map --> (service_name, service_info)
 *                                         |
 *                                         v
 *              service_info -->  (service , method_map)
 *                                               |
 *                                               v
 *                 method_map  -->  (method_name , method_description)
 */

/**
 * @brief 注册服务对象及其方法到 service_map ,以便服务端能够处理客户端的 RPC 请求
 */
void KrpcProvider::NotifyService(google::protobuf::Service* service){
    // 服务端需要知道客户端想要调用的服务对象和方法
    ServiceInfo service_info;

    // 参数类型为 google::protobuf::Service，因为所有由protobuf 生成的服务类都继承自 google::protobuf::Service
    // 这样可以通过基类指针指向子类对象，实现动态多态
    // 返回protobuf 生成的服务类的描述信息
    const google::protobuf::ServiceDescriptor *psd = service->GetDescriptor();

    /// 通过 ServiceDescriptor 获取该服务类中定义的方法列表并进行相应的注册和管理
    std::string service_name = psd->name();  // 获取服务的名字
    int method_count = psd->method_count();  // 获取服务端对象 service 的方法数量
    std::cout << "service_name = " << service_name << std::endl;
    // 遍历服务中的所有方法，并注册到服务信息中
    for(int i = 0; i < method_count; ++i) {
        // 获取服务中的方法描述
        const google::protobuf::MethodDescriptor *pmd = psd->method(i);
        std::string method_name = pmd->name();
        std::cout << "method_name = " << method_name << std::endl;
        // 将方法名和方法描述存入 method_map
        service_info.method_map.emplace(method_name, pmd);
    }
    service_info.service = service;   // 保存服务对象
    service_map.emplace(service_name, service_info);  // 将服务名和服务信息存入 service_map
}

/**
 * @brief 启动 RPC 服务节点，开始提供远程网络调用服务
 * @details 注册 zookeeper 服务 并 开启事件循环监听
 */
void KrpcProvider::Run() {
    // 读取配置文件中的 RPC 服务器 IP 和 端口
    std::string ip = KrpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    int port = atoi(KrpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    /**
     * @brief muduo 事件监听
     */
    // 使用muduo网络库，创建地址对象
    muduo::net::InetAddress address(ip, port);
    // 创建TcpServer对象
    std::shared_ptr<muduo::net::TcpServer> server = std::make_shared<muduo::net::TcpServer>(&event_loop, address, "KrpcProvider");
    // 绑定连接回调和消息回调，分离网络连接业务和消息处理业务
    server->setConnectionCallback(std::bind(&KrpcProvider::OnConnection, this, std::placeholders::_1));
    server->setMessageCallback(std::bind(&KrpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置muduo的线程数量
    server->setThreadNum(4);
    /**
     * @brief zookeeper 服务注册 ，将 service_name 和 method_name 注册到 zookkeeper 服务器上
     */
    // 将当前RPC节点上要发布的服务全部注册到 ZooKeeper 上，让 RPC 客户端可以在 ZooKeeper 上发现服务
    ZkClient zkclient;
    zkclient.Start();  // 连接zookeeper 服务器
    /// server_name 为永久节点， method_name 为临时节点
    for(auto &sp : service_map){
        // service_name 在 zookeeper 中的目录是 "/" + service_name <永久节点>
        std::string service_path = "/" + sp.first;
        // 创建服务节点
        zkclient.Create(service_path.c_str(), nullptr, 0);
        for(auto &mp : sp.second.method_map){
            // method_name 在ZooKeeper中的目录是"/" + service_name/method_name  <临时节点>
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port); // 将IP和端口信息存入节点数据
            // ZOO_EPHEMERAL 表示这个节点是临时节点，在客户端断开连接后，ZooKeeper会自动删除这个节点
            zkclient.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }
    // RPC 服务端准备启动 打印信息
    std::cout << "RpcProvider start service at ip: " << ip << " port: " << port << std::endl;
    // 启动网络服务
    server->start();
    event_loop.loop(); // 开启事件循环
}

/**
 * @brief 连接回调函数 处理客户端连接事件
 * @param conn 连接
 */
void KrpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn){
    if(!conn->connected()){
        conn->shutdown();      // 如果连接关闭，则断开连接
    }
}

/**
 * @brief 消息回调函数，处理客户端发送的 RPC 请求
 * @details 从缓冲区读取并解析请求信息，获取请求中的 service 对象和 method 对象
 */
void KrpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn,
               muduo::net::Buffer* buffer, muduo::Timestamp receive_time){
    std::cout << "OnMessage" << std::endl;
    /// 从网络缓冲区读取远程 RPC 调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();
    // 使用 protobuf 的CodeInputStream 反序列化 RPC 请求
    google::protobuf::io::ArrayInputStream raw_input(recv_buf.data(), recv_buf.size());
    google::protobuf::io::CodedInputStream coded_input(&raw_input);

    /*
     * Protobuf格式-> {RpcHeader:[header_size, (service_name, method_name, args_size)], args }
     */
    uint32_t header_size{};       // 头部长度
    coded_input.ReadVarint32(&header_size); // 解析 header_size

    // 根据 header_size 读取数据头的原始字符流，反序列化数据，得到RPC 请求的详细信息
    std::string rpc_header_str;   // 头信息字符串(缓冲区)
    Krpc::RpcHeader krpcHeader;   // krpc头部
    std::string service_name;     // 服务对象名
    std::string method_name;      // 方法名
    uint32_t args_size{};         // 函数参数长度

    // 设置读取限制
    google::protobuf::io::CodedInputStream::Limit msg_limit = coded_input.PushLimit(header_size);
    coded_input.ReadString(&rpc_header_str, header_size);
    // 恢复之前的限制，以便安全的继续读取其他数据
    coded_input.PopLimit(msg_limit);
    // 读取头部信息 包含 服务名、方法名、参数长度
    if (krpcHeader.ParseFromString(rpc_header_str)) {
        service_name = krpcHeader.service_name();
        method_name = krpcHeader.method_name();
        args_size = krpcHeader.args_size();
    } else {
        KrpcLogger::Error("read args error");
        return;
    }

    // 读取 RPC 参数 ，直接读取 args_size 长度的字符串数据
    std::string args_str;
    bool read_args_success = coded_input.ReadString(&args_str, args_size);
    if(!read_args_success) {
        KrpcLogger::Error("read args error");
        return;
    }

    /// 从 service_map 中获取 service 对象和 method 对象
    auto it = service_map.find(service_name);
    if(it == service_map.end()){
        std::cout << service_name << " is not exits!" << std::endl;
        return;
    }
    auto mit = it->second.method_map.find(method_name);
    if(mit == it->second.method_map.end()) {
        std::cout << service_name << "." << method_name << "is not exist!" << std::endl;
        return;
    }
    // 获取服务对象和服务方法
    google::protobuf::Service *service = it->second.service;
    const google::protobuf::MethodDescriptor * method = mit->second;

    /// 生成 RPC 方法调用请求的request和响应的response参数
    // 动态创建请求对象
    google::protobuf::Message * request = service->GetRequestPrototype(method).New();
    // 解析请求参数
    if(!request->ParseFromString(args_str)) {
        std::cout << service_name << "." << method_name << " parse error!" << std::endl;
        return;
    }
    // 动态创建响应对象
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();
    /// 绑定回调函数 用于在方法调用完成后发送响应
    /// 相当于执行 void RpcProvider::SendRpcResponse(conn, response)
    google::protobuf::Closure *done = google::protobuf::NewCallback<KrpcProvider,
                                                        const muduo::net::TcpConnectionPtr &,
                                                        google::protobuf::Message *>(this,
                                                                                     &KrpcProvider::SendRpcResponse,
                                                                                     conn, response);
    // 在框架上根据远端 RPC 请求，调用当前 RPC 节点上发布的方法
    service->CallMethod(method, nullptr, request, response, done); // 调用服务方法
}

/**
 * @brief 发送 PRC 响应给客户端
 * @param conn 连接
 * @param response 服务端响应
 */
void KrpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response){
    std::string response_str;
    // 如果序列化成功，通过网络把 PRC 方法执行的结果返回给客户端调用方
    if(response->SerializeToString(&response_str)) {
        conn->send(response_str);
    } else {
        std::cout << "Serialize Response error!" << std::endl;
    }
    // conn->shutdown(); // 模拟HTTP短链接，由RpcProvider主动断开连接
}
/**
 * @brief 析构函数 退出事件循环
 */
KrpcProvider::~KrpcProvider() {
    std::cout << "~KrpcProvider()" << std::endl;
    event_loop.quit();  // 退出事件循环
}
