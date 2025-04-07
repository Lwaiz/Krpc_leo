/**
  ******************************************************************************
  * @file           : Krpc_Logger.h
  * @author         : 18483
  * @brief          : None
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */


#ifndef KRPC_KRPC_LOGGER_H
#define KRPC_KRPC_LOGGER_H

#include <glog/logging.h>
#include <string>

/**
 * @brief 采用RAII思想 封装 Google glog 日志系统
 */
class KrpcLogger {
public:
    /**
     * @brief 构造函数  自动初始化 glog
     * @param argv0
     */
    explicit KrpcLogger(const char *argv0){
        google::InitGoogleLogging(argv0);
        FLAGS_colorlogtostderr = true;  // 启用色彩日志
        FLAGS_logtostderr = true;       // 默认输出标准错误
    }
    ~KrpcLogger(){
        google::ShutdownGoogleLogging();
    }
    /**
     * @brief 提供静态日志输出方法
     * @param message
     */
    static void Info(const std::string & message){
        LOG(INFO) << message;
    }
    static void Warning(const std::string & message){
        LOG(WARNING) << message;
    }
    static void Error(const std::string & message){
        LOG(ERROR) << message;
    }
    static void Fatal(const std::string & message){
        LOG(FATAL) << message;
    }
private:
    // 禁用拷贝构造和重载赋值运算符
    KrpcLogger(const KrpcLogger&) = delete;
    KrpcLogger& operator=(const KrpcLogger&) = delete;
};

#endif //KRPC_KRPC_LOGGER_H
