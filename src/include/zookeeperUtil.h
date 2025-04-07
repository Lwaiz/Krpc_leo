/**
  ******************************************************************************
  * @file           : zookeeperUtil.h
  * @author         : 18483
  * @brief          : zookeeper 封装
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */

#ifndef KRPC_ZOOKEEPERUTIL_H
#define KRPC_ZOOKEEPERUTIL_H

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>


/**
 * @brief 封装 zk 客户端
 */
class ZkClient {
public:
    ZkClient();
    ~ZkClient();
    /**
     * @brief zkclient 启动连接 zkserver
     */
    void Start();
    /**
     * @brief 在 zkserver 中根据指定的 path 创建一个节点
     */
    void Create(const char* path, const char* data, int datalen, int state = 0);
    /**
     * @brief 根据节点路径获取 znode 节点值
     */
    std::string GetData(const char* path);
private:
    /// zk 的客户端句柄
    zhandle_t* m_zhandle;
};

#endif //KRPC_ZOOKEEPERUTIL_H

