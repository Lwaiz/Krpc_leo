/**
  ******************************************************************************
  * @file           : Krpcconfig.h
  * @author         : 18483
  * @brief          : 配置模块
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */


#ifndef KRPC_KRPCCONFIG_H
#define KRPC_KRPCCONFIG_H

#include <unordered_map>
#include <string>

/**
 * @brief Krpc 配置类
 */
class KrpcConfig {
public:
    /**
     * @brief 加载配置文件
     */
    void LoadConfigFile(const char* config_file);
    /**
     * @brief 查找 key 对应的 value
     */
    std::string Load(const std::string & key);
private:
    /**
     * @brief 去掉字符串前后的空格
     */
    void Trim(std::string & read_buf);
private:
    /// 配置存储容器
    std::unordered_map<std::string, std::string> config_map;
};

#endif //KRPC_KRPCCONFIG_H
