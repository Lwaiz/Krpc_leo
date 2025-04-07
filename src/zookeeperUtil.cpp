/**
  ******************************************************************************
  * @file           : zookeeperUtil.cpp
  * @author         : 18483
  * @brief          : zookeeper 实现
  * @attention      : None
  * @date           : 2025/4/6
  ******************************************************************************
  */

#include "zookeeperUtil.h"
#include "Krpc_Application.h"
#include <mutex>
#include "Krpc_Logger.h"
#include <condition_variable>

std::mutex cv_mutex;        // 全局锁 用于保护共享变量的线程安全
std::condition_variable cv; // 条件变量 用于线程间的通信
bool is_connected = false;  // 标记 ZooKeeper 客户端是否连接成功

/**
 * @brief 全局的 watcher 观察器，用于接收 ZooKeeper 服务器的通知
 */
void global_watcher(zhandle_t* zh, int type, int status, const char* path, void* watcherCtx) {
    // 回调消息类型 和会话相关的事件
    if(type == ZOO_SESSION_EVENT) {
        // zookeeper客户端和服务器连接成功
        if(status == ZOO_CONNECTED_STATE) {
            std::lock_guard<std::mutex> lock(cv_mutex);  // 加锁保护
            is_connected = true;  // 标记连接成功
        }
    }
    cv.notify_all();   // 通知所有等待的线程
}

/**
 * @brief 构造函数 初始化 zookeeper 客户端句柄为空
 */
ZkClient::ZkClient() : m_zhandle(nullptr){}

/**
 * @brief 析构函数 关闭 zookeeper 连接
 */
ZkClient::~ZkClient() {
    if(m_zhandle != nullptr) {
        zookeeper_close(m_zhandle);
    }
}
/*
 * 使用 zookeeper_mt 多线程版本
 * zookeeper 的API 客户端提供三个线程
 * 1.API调用线程
 * 2.网络I/O线程 (使用pthread_create和poll)
 * 3.watcher回调线程 (使用pthread_create)
 */

/**
 * @brief 启动 zookeeper 客户端，连接 zookeeper 服务器
 */
void ZkClient::Start() {
    // 从配置文件中读取 Zookeeper服务器的 IP 和 端口
    std::string host = KrpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = KrpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;  // 拼接连接字符串

    /// 使用 zookeeper_init 初始化一个 Zookeeper 客户端对象，异步建立与服务器的连接
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 6000, nullptr, nullptr, 0);
    if(nullptr == m_zhandle) {  // 初始化失败
        LOG(ERROR) << "zookeeper_init error";
        exit(EXIT_FAILURE);
    }
    // 等待连接成功
    std::unique_lock<std::mutex> lock(cv_mutex);
    cv.wait(lock, [] {return is_connected;}); // 阻塞等待 直到连接成功
    LOG(INFO) << "zookeeper_init success";  // 连接成功
}

/**
 * @brief 创建 zookeeper 节点
 */
void ZkClient::Create(const char *path, const char *data, int datalen, int state) {
    char path_buffer[128];   // 用于存储创建的节点路径
    int bufferlen = sizeof(path_buffer);
    // 检查节点是否已经存在
    int flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (flag == ZNONODE) {  // 节点不存在
        // 创建指定的 ZooKeeper 节点
        flag = zoo_create(m_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if(flag == ZOK) {   // 节点创建成功
            LOG(INFO) << "znode create success ... path: " << path;
        } else {
            LOG(ERROR) << "znode create failed ... path: " << path;
            exit(EXIT_FAILURE); // 退出程序
        }
    }
}

/**
 * @brief 获取 zookeeper 节点的数据
 */
std::string ZkClient::GetData(const char *path) {
    char buf[64];  // 用于存储节点数据
    int bufferlen = sizeof(buf);
    // 获取指定节点的数据
    int flag = zoo_get(m_zhandle, path, 0, buf, &bufferlen, nullptr);
    if(flag != ZOK) {
        LOG(ERROR) << "zoo_get error";
        return "";    // 获取失败返回空字符串
    } else {
        return buf;   // 获取成功返回节点数据
    }
    return "";        // 默认返回空字符串
}







