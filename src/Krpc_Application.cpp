/**
  ******************************************************************************
  * @file           : Krpc_Application.cpp
  * @author         : 18483
  * @brief          : None
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */

#include <iostream>
#include "Krpc_Application.h"
#include <cstdlib>
#include <unistd.h>

// 全局配置对象
KrpcConfig KrpcApplication::m_config;
std::mutex KrpcApplication::m_mutex;   // 互斥锁
// 单例对象指针，初始化为空
KrpcApplication* KrpcApplication::m_application = nullptr;

/**
 * @brief 初始化函数，解析命令行参数 并加载配置文件
 * @details 执行可执行程序的时候必须跟上 -i 后面存放 zookeeper 服务器的 ip 和 port 及服务器 ip 和 port，
 * 并且这里还会调用 m_config.LoadConfigFile(config_file.c_str()); 将服务器的 ip 和 port 进行存放，
 * 并且对其字符串空字符进行删除
 */
void KrpcApplication::Init(int argc, char **argv) {
    // 如果命令行参数少于2个，说明没有指定配置文件
    if(argc < 2){
        std::cout << "格式： command -i <配置文件路径>" << std::endl;
        exit(EXIT_FAILURE); // 退出程序
    }
    int o;
    std::string config_file;
    // 使用 getopt 解析命令行参数， -i表示指定配置文件
    while(-1 != (o = getopt(argc, argv, "i:"))) {
        switch(o) {
            case 'i':   // 如果参数是-i，后面的值就是配置文件的路径
                config_file = optarg;
                break;
            case '?':   // 如果出现未知参数（不是-i），提示正确格式并退出
                std::cout << "格式： command -i <配置文件路径>" << std::endl;
                exit(EXIT_FAILURE);
                break;
            case ':':   // 如果-i后面没有跟参数，提示正确格式并退出
                std::cout << "格式： command -i <配置文件路径>" << std::endl;
                exit(EXIT_FAILURE);
                break;
            default:
                break;
        }
    }
    // 加载配置文件 存入 m_config 的 config_map 中
    m_config.LoadConfigFile(config_file.c_str());
}

/**
 * @brief 获取单例对象的引用 保证全局只有一个实例
 */
KrpcApplication &KrpcApplication::GetInstance() {
    // lock_guard 是互斥锁自动管理工具类，构造时自动调用 m_mutex.lock() ，作用域结束时会自动调用 m_mutex.unlock()
    std::lock_guard<std::mutex> lock(m_mutex);  // 加锁 保证线程安全
    // 如果未创建单例对象，创建
    if(m_application == nullptr) {
        m_application = new KrpcApplication();
        // 注册 atexit 函数，程序退出时自动销毁单例对象
        atexit(deleteInstance);  // atexit 是一个 标准库函数 作用是：注册一个函数，在 main() 结束或程序调用 exit() 时被自动执行
    }
    return *m_application; // 返回单例对象的引用
}

/**
 * @brief 程序退出时自动调用的函数，用于销毁单例对象
 */
void KrpcApplication::deleteInstance() {
    if(m_application){
        delete m_application;
    }
}
/**
 * @brief 获取全局配置对象的引用
 */
KrpcConfig& KrpcApplication::GetConfig() {
    return m_config;
}








