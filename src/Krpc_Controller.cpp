/**
  ******************************************************************************
  * @file           : Krpc_Controller.cpp
  * @author         : 18483
  * @brief          : Krpc_Controller 控制器
  * @attention      : None
  * @date           : 2025/4/6
  ******************************************************************************
  */

#include "Krpc_Controller.h"

/**
 * @brief 构造函数 初始化控制器状态
 */
KrpcController::KrpcController() {
    m_failed = false;  // 初始状态为 未失败
    m_errText = "";    // 错误信息初始为空
}

/**
* 重置控制器状态 将失败标志和失败信息清空
*/
void KrpcController::Reset() {
    m_failed = false;
    m_errText = "";
}

/**
 * @brief 判断当前 RPC 调用是否失败
 */
bool KrpcController::Failed() const {
    return m_failed;
}

/**
 * @brief 获取错误信息
 */
std::string KrpcController::ErrorText() const {
    return m_errText;
}

/**
 * @brief 设置 RPC 调用失败，并记录失败原因
 */
void KrpcController::SetFailed(const std::string &reason) {
    m_failed = true;     // 设置失败标志
    m_errText = reason;  // 记录失败原因
}


/// 以下功能未实现，是RPC服务端提供的取消功能
// 开始取消RPC调用（未实现）
void KrpcController::StartCancel() {
    // 目前为空，未实现具体功能
}

// 判断RPC调用是否被取消（未实现）
bool KrpcController::IsCanceled() const {
    return false;  // 默认返回false，表示未被取消
}

// 注册取消回调函数（未实现）
void KrpcController::NotifyOnCancel(google::protobuf::Closure* callback) {
    // 目前为空，未实现具体功能
}
