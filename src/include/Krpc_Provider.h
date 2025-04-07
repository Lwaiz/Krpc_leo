/**
  ******************************************************************************
  * @file           : Krpc_Provider.h
  * @author         : 18483
  * @brief          : KrpcProvider 服务端核心类
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */


#ifndef KRPC_KRPC_PROVIDER_H
#define KRPC_KRPC_PROVIDER_H

#include "google/protobuf/service.h"
#include "zookeeperUtil.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <google/protobuf/descriptor.h>
#include <functional>
#include <string>
#include <unordered_map>

class KrpcProvider {
public:
    /**
     * @brief 供外部使用，用于发布RPC方法的函数接口
     * @param service 服务
     */
    void NotifyService(google::protobuf::Service* service);
    /**
     * @brief 析构函数
     */
    ~KrpcProvider();
    /**
     * @brief 启动RPC服务节点，开始提供RPC远程网络调用服务
     */
    void Run();
private:
    /**
     * @brief 服务信息结构体 存储服务对象和方法map
     * @details <服务对象， 方法:<方法名, 方法描述>>
     */
    struct ServiceInfo{
        // 服务对象
        google::protobuf::Service* service;
        // 存放方法的 method_map <方法名, 方法描述>
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> method_map;
    };

    /**
     * @brief 连接回调函数 处理客户端连接事件
     * @param conn
     */
    void OnConnection(const muduo::net::TcpConnectionPtr& conn);
    /**
     * @brief 消息回调函数，处理客户端发送的RPC请求
     * @param conn
     * @param buffer
     * @param receive_time
     */
    void OnMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buffer, muduo::Timestamp receive_time);
    /**
     * @brief 响应回调函数 发送 PRC 响应给客户端
     * @param conn
     * @param response
     */
    void SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response);

private:
    /// 事件循环
    muduo::net::EventLoop event_loop;
    /// 服务map 保存服务对象和 服务信息  <service_name, service_info>
    std::unordered_map<std::string, ServiceInfo> service_map;
};

/*
 *    service_map --> (service_name, service_info)
 *                                         |
 *                                         v
 *              service_info -->  (service , method_map)
 *                                               |
 *                                               v
 *                 method_map -->   (method_name , method_description)
 */

#endif //KRPC_KRPC_PROVIDER_H
