/**
  ******************************************************************************
  * @file           : Krpc_Controller.h
  * @author         : 18483
  * @brief          : RPC 调用的控制器
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */


#ifndef KRPC_KRPC_CONTROLLER_H
#define KRPC_KRPC_CONTROLLER_H

#include <google/protobuf/service.h>
#include <string>

/**
 * @brief 用于描述 RPC 调用的控制器
 * @details 跟踪 RPC 方法调用的状态、错误信息 并 提供控制功能(如取消调用)
 */
class KrpcController : public google::protobuf::RpcController {
public:
    /**
     * @brief 构造函数，初始化控制器状态
     */
    KrpcController();
    /**
     * @brief 重置控制器状态，将失败标志和错误信息清空
     */
    void Reset();
    /**
     * @brief 判断当前RPC调用是否失败
     */
    bool Failed() const;
    /**
     * @brief 获取错误信息
     */
    std::string ErrorText() const;
    /**
     * @brief 设置RPC调用失败，并记录失败原因
     */
    void SetFailed(const std::string &reason);

    /// 未实现
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    /// RPC 方法执行过程中的状态
    bool m_failed;
    /// RPC 方法执行过程中的错误信息
    std::string m_errText;
};


#endif //KRPC_KRPC_CONTROLLER_H
