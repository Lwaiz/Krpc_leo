/**
  ******************************************************************************
  * @file           : Krpc_Channel.cpp
  * @author         : 18483
  * @brief          : Channel 客户端类
  * @attention      : None
  * @date           : 2025/4/7
  ******************************************************************************
  */

#include "Krpc_Channel.h"
#include "Krpcheader.pb.h"
#include "zookeeperUtil.h"
#include "Krpc_Application.h"
#include "Krpc_Controller.h"
#include <memory>
#include <error.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "Krpc_Logger.h"

/// 全局互斥锁 用于保护共享数据的线程安全
std::mutex g_data_mutex;

/**
 * @brief 构造函数 支持延迟连接
 */
KrpcChannel::KrpcChannel(bool connectNow)
        :m_clientfd(-1), m_idx(0) {
    if(!connectNow) { // 不需要立即连接
        return;
    }
    // 尝试连接服务器 最多重试 3 次
    auto rt = newConnect(m_ip.c_str(), m_port);
    int count = 3;
    while(!rt && count--) {
        rt = newConnect(m_ip.c_str(), m_port);
    }
}


/**
 * @brief  RPC 调用的核心方法
 * @details 负责将客户端的请求序列化并发送到服务器，同时接收服务端的响应
 */
void KrpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
                             ::google::protobuf::RpcController *controller,
                             const ::google::protobuf::Message *request,
                             ::google::protobuf::Message *response,
                             ::google::protobuf::Closure *done) {
    // 如果客户端socket未初始化
    if(-1 == m_clientfd){
        /// 获取服务对象名和方法名
        const google::protobuf::ServiceDescriptor *sd = method->service();
        service_name = sd->name();    // 服务名
        method_name = method->name(); // 方法名

        /// 客户端查询 ZooKeeper 找到提供该服务的服务器地址
        ZkClient zkCli;
        zkCli.Start();   // 连接zookeeper 服务器
        // 查询服务地址
        std::string host_data = QueryServiceHost(&zkCli, service_name, method_name, m_idx);
        m_ip = host_data.substr(0, m_idx);  // 提取IP地址
        std::cout << "ip: " << m_ip << std::endl;
        m_port = atoi(host_data.substr(m_idx + 1, host_data.size() - m_idx).c_str()); // 端口号
        std::cout << "port: " << m_port << std::endl;

        /// 尝试连接服务器
        auto rt = newConnect(m_ip.c_str(), m_port);
        if(!rt) {
            LOG(ERROR) << "connect server error";
            return;
        } else {
            LOG(ERROR) << "connect server success";
        }
    }

    /// 将请求参数序列化为字符串，并计算其长度
    uint32_t args_size{};
    std::string args_str;
    if(request->SerializeToString(&args_str)) {  // 序列化请求参数
        args_size = args_str.size();  // 获取序列化后的长度
    } else {
        controller->SetFailed("serialize request fail!");
        return;
    }

    /// 定义 RPC 请求的头部信息
    Krpc::RpcHeader krpcheader;
    krpcheader.set_service_name(service_name);
    krpcheader.set_method_name(method_name);
    krpcheader.set_args_size(args_size);

    /// 将 RPC 头部信息序列化为字符串，并计算其长度
    uint32_t header_size = 0;
    std::string rpc_header_str;
    if(krpcheader.SerializeToString(&rpc_header_str)) { // 序列化头部信息
        header_size = rpc_header_str.size();   // 获取头部序列化后的长度
    } else {
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    /*
     * RPC_Str --> { header:[header_size, (service_name, method_name, args_size)], args_str}
     */

    /// 将头部长度和头部信息拼接成完整的 RPC 请求报文
    std::string send_rpc_str;
    {
        google::protobuf::io::StringOutputStream string_output(&send_rpc_str);
        google::protobuf::io::CodedOutputStream coded_output(&string_output);
        coded_output.WriteVarint32(static_cast<uint32_t>(header_size)); // 写入头部信息
        coded_output.WriteString(rpc_header_str);  // 写入头部信息
    }
    send_rpc_str += args_str;   // 拼接请求参数

    /// 发送 RPC 请求到服务器
    if(-1 == send(m_clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0)) {
        close(m_clientfd); // 发送失败 关闭socket
        char errtxt[512] = {};
        std::cout << "SEND error: " << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
        controller->SetFailed(errtxt);
        return;
    }

    /// 接收服务器的响应
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if(-1 == (recv_size = recv(m_clientfd, recv_buf, 1024, 0))) {
        char errtxt[512] = {};
        std::cout << "RECV error: " << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
        controller->SetFailed(errtxt);
        return;
    }

    /// 反序列化接收到的响应数据 为 response 对象
    if(!response->ParseFromArray(recv_buf, recv_size)) {
        close(m_clientfd); // 反序列化失败 关闭socket
        char errtxt[512] = {};
        std::cout << "PARSE error: " << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
        controller->SetFailed(errtxt);
        return;
    }
    // 关闭连接
    close(m_clientfd);
}

/**
 * @brief 创建新的 socket 连接
 */
bool KrpcChannel::newConnect(const char *ip, uint16_t port) {
    /// 创建socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == clientfd) {
        char errtxt[512] = {};
        std::cout << "SOCKET error: " << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
        LOG(ERROR) << "socket error: " << errtxt; // 记录错误日志
        return false;
    }

    /// 设置服务器地址信息
    struct sockaddr_in service_addr;
    service_addr.sin_family = AF_INET;   // IPv4地址族
    service_addr.sin_port = htons(port);
    service_addr.sin_addr.s_addr = inet_addr(ip);

    /// 尝试连接服务器
    if(-1 == connect(clientfd, (struct sockaddr*)&service_addr, sizeof(service_addr))) {
        close(clientfd);
        char errtxt[512] = {};
        std::cout << "CONNECT error: " << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
        LOG(ERROR) << "connect error: " << errtxt; // 记录错误日志
        return false;
    }

    /// 保存 socket 文件描述符
    m_clientfd = clientfd;
    return true;
}

/**
 * @brief 从 ZooKeeper 查询服务地址
 */
std::string KrpcChannel::QueryServiceHost(ZkClient *zkclient, std::string service_name,
                                          std::string method_name, int &idx) {
    // 构造 ZooKeeper 路径
    std::string method_path = "/" + service_name + "/" + method_name;
    std::cout << "method_path: " << method_path << std::endl;

    std::unique_lock<std::mutex> lock(g_data_mutex);  // 加锁
    std::string host_data_1 = zkclient->GetData(method_path.c_str());  // 获取数据
    lock.unlock();  // 解锁

    if(host_data_1 == "") {
        LOG(ERROR) << method_path + " is not exist!";
        return " ";
    }

    idx = host_data_1.find(":"); // IP 和 端口分隔符
    if(idx == -1) {
        LOG(ERROR) << method_path + " address is invalid!";
        return " ";
    }
    return host_data_1;  // 返回服务地址
}



