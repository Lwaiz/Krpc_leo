/**
  ******************************************************************************
  * @file           : Krpc_Channel.h
  * @author         : 18483
  * @brief          : None
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */


#ifndef KRPC_KRPC_CHANNEL_H
#define KRPC_KRPC_CHANNEL_H

#include <google/protobuf/service.h>
#include "zookeeperUtil.h"

/**
 * @brief 给客户端进行方法调用的时候，统一接收
 * @details 继承自google::protobuf::RpcChannel
 */
class KrpcChannel : public google::protobuf::RpcChannel {
public:
    /**
     * @brief 构造函数
     * @param connectNow 是否立即建立连接
     */
    KrpcChannel(bool connectNow);
    /**
     * @brief 虚析构函数
     */
    virtual ~KrpcChannel() {}
    /**
     * @brief RPC 调用的核心方法
     * @details 负责将客户端的请求序列化并发送到服务器，同时接收服务端的响应
     * @param method 客户端要调用的方法
     * @param controller rpc 控制器
     * @param request  客户端请求
     * @param response 服务端响应
     * @param done
     */
    void CallMethod(const ::google::protobuf::MethodDescriptor * method,
                    ::google::protobuf::RpcController * controller,
                    const ::google::protobuf::Message * request,
                    ::google::protobuf::Message * response,
                    ::google::protobuf::Closure * done) override;
private:
    /**
     * @brief 建立新连接
     */
    bool newConnect(const char *ip, uint16_t port);
    /**
     * @brief 从 ZooKeeper 查询服务地址
     * @param zkclient zk 客户端
     * @param service_name 服务名
     * @param method_name  方法名
     * @param idx      ip 和 port 分隔符下标
     */
    std::string QueryServiceHost(ZkClient *zkclient, std::string service_name,
                                 std::string method_name, int &idx);
private:
    /// 存放客户端套接字
    int m_clientfd;
    std::string service_name;
    std::string method_name;
    std::string m_ip;
    uint16_t m_port;
    /// 用于划分服务器 ip 和 port 的下标
    int m_idx;
};


#endif //KRPC_KRPC_CHANNEL_H
