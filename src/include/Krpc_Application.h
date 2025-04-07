/**
  ******************************************************************************
  * @file           : Krpc_Application.h
  * @author         : 18483
  * @brief          : Krpc 基础类 初始化
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */


#ifndef KRPC_KRPC_APPLICATION_H
#define KRPC_KRPC_APPLICATION_H

#include "Krpc_Config.h"
#include "Krpc_Controller.h"
#include "Krpc_Channel.h"
#include <mutex>

/**
 * @brief Krpc 基础类 负责框架的一些初始化操作
 */
class KrpcApplication {
public:
    /**
     * @brief   初始化函数
     * @details 解析命令行参数 并加载配置文件
     */
    static void Init(int argc, char** argv);
    /**
     * @brief 获取单例对象的引用 保证全局只有一个实例
     */
    static KrpcApplication & GetInstance();
    /**
     * @brief 程序退出时自动调用的函数，用于销毁单例对象
     */
    static void deleteInstance();
    /**
     * @brief 获取全局对象的引用
     */
    static KrpcConfig& GetConfig();

private:
    /// 单例模式 私有构造、析构 ，禁用拷贝构造、移动构造
    KrpcApplication(){}
    ~KrpcApplication(){}
    KrpcApplication(const KrpcApplication &) = delete;
    KrpcApplication(KrpcApplication &&) = delete;

private:
    static KrpcConfig m_config;             // 全局配置对象
    static KrpcApplication* m_application;  // 全局唯一单例访问对象
    static std::mutex m_mutex;
};



#endif //KRPC_KRPC_APPLICATION_H
