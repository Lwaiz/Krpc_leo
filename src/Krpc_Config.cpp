/**
  ******************************************************************************
  * @file           : Krpcconfig.cpp
  * @author         : 18483
  * @brief          : 配置模块
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */


#include "Krpc_Config.h"
#include <memory>

/// 加载配置文件，解析配置文件中的键值对
void KrpcConfig::LoadConfigFile(const char *config_file) {
    // 使用智能指针管理文件指针，确保文件在退出时自动关闭
    std::unique_ptr<FILE, decltype(&fclose)> pf(
            fopen(config_file, "r"), // 打开配置文件
            &fclose              // 文件关闭函数
            );
    if(pf == nullptr){              // 如果文件打开失败
        exit(EXIT_FAILURE);   // 退出程序
    }
    char buf[1024];                 // 存储从文件中读取的每一行内容
    // 使用pf.get()方法获取原始指针，逐行读取文件内容
    while(fgets(buf, 1024, pf.get()) != nullptr) {
        std::string read_buf(buf); // 将读取的内容转换为字符串
        Trim(read_buf);           // 去掉字符串前后的空格

        // 忽略注释行和空行
        if(read_buf[0] == '#' || read_buf.empty()) continue;
        // 查找键值对的分隔符'='
        int index = read_buf.find('=');
        if(index == -1) continue;  // 如果没有找到'=',跳过该行
        //提取键 key
        std::string key = read_buf.substr(0, index);
        Trim(key);  // 去掉key前后的空格

        // 查找行尾的换行符
        int endindex = read_buf.find('\n', index);
        // 提取值 value， 并去掉换行符
        std::string value = read_buf.substr(index + 1, endindex - index -1);
        Trim(value);
        // 将键值对存入配置 map 中
        config_map.insert({key, value});
    }
}

/// 根据 key 查找对应的 value
std::string KrpcConfig::Load(const std::string &key) {
    auto it = config_map.find(key);
    if(it == config_map.end()){
        return "";     // 未找到，返回空字符串
    }
    return it->second; // 返回对应的 value
}

/// 去掉字符串前后的空格
void KrpcConfig::Trim(std::string &read_buf) {
    // 去掉字符串前面的空格
    int index = read_buf.find_first_not_of(' ');
    if(index != -1) {
        read_buf = read_buf.substr(index, read_buf.size() - index); // 截取字符串
    }
    // 由于windows下文件换行符是'\r\n' , linux下换行符是'\n'
    // windows下建立的config文件解析过程中 可能出现问题
    index = read_buf.find_last_not_of('\r');
    if(index != -1) {
        read_buf = read_buf.substr(0, index + 1); // 截取字符串
    }
    // 去掉字符串后面的空格
    index = read_buf.find_last_not_of(' ');
    if(index != -1) {
        read_buf = read_buf.substr(0, index + 1); // 截取字符串
    }

}