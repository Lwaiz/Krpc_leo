/**
  ******************************************************************************
  * @file           : Kclient.cpp
  * @author         : 18483
  * @brief          : 客户端实现
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */

#include "Krpc_Application.h"
#include "../user.pb.h"
#include "Krpc_Controller.h"
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include "Krpc_Logger.h"

/**
 * @brief 发送 RPC 请求的函数 模拟客户端调用远程服务
 */
void send_request(int thread_id, std::atomic<int> &success_count, std::atomic<int> &fail_count){
    // 创建一个UserServiceRpc_Stub对象，用于调用远程的 RPC 方法
    Kuser::UserServiceRpc_Stub stub(new KrpcChannel(false));

    // 设置 RPC方法的请求参数
    Kuser::LoginRequest request;
    request.set_name("leo");    // 设置用户名
    request.set_pwd("123456");  // 设置密码

    // 定义 RPC 方法的响应参数
    Kuser::LoginResponse response;
    KrpcController controller;  // 创建控制器对象，用于处理 RPC 调用过程中的错误

    // 调用远程的 Login 方法
    stub.Login(&controller, &request, &response, nullptr);

    // 检查 RPC 调用是否成功
    if(controller.Failed()){
        std::cout << controller.ErrorText() << std::endl;
        fail_count++;  // 失败计数加 1
    } else {           // 如果调用成功
        if(0 == response.result().errcode()) {   // 检查响应中的错误码
            LOG(INFO) << "Thread " << thread_id << " login response success: " << response.success();
            success_count++; // 成功计数 +1
        } else {       // 如果响应中有错误
            std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
            fail_count++; // 失败计数 +1
        }
    }
}

int main(int argc, char **argv) {
    // 初始化 RPC 框架，解析命令行参数 并加载配置文件
    KrpcApplication::Init(argc, argv);

    // 创建日志对象
    KrpcLogger logger("LeoRPC");
    const int thread_count = 10;      // 并发线程数
    const int request_per_thread = 100; // 每个线程发送的请求数

    std::vector<std::thread> threads;     // 存储线程对象的容器
    std::atomic<int> success_count(0);  // 成功请求的计数器
    std::atomic<int> fail_count(0);     // 失败请求的计数器

    // 记录测试开始时间
    auto start_time = std::chrono::high_resolution_clock::now();

    // 启动多线程进行并发测试
    for(int i = 0; i < thread_count; i++){
        threads.emplace_back([argc, argv, i, &success_count, & fail_count, request_per_thread] (){
            for(int j = 0; j < request_per_thread; j++){
                // 每个线程发送指定数量的请求
                send_request(i, success_count, fail_count);
            }
        });
    }

    // 等待所有线程执行完毕
    for(auto &t : threads){
        t.join();
    }

    // 记录测试结束时间
    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算测试耗时
    std::chrono::duration<double> elapsed = end_time - start_time;

    // 输出统计结果
    LOG(INFO) << "Total request: " << thread_count * request_per_thread; // 请求总数
    LOG(INFO) << "Success count: " << success_count;  // 成功请求数
    LOG(INFO) << "Fail count: " << fail_count;        // 失败请求数
    LOG(INFO) << "Elapsed time: " << elapsed.count() << " seconds"; // 耗时
    LOG(INFO) << "QPS: " << (thread_count * request_per_thread) / elapsed.count(); // // 计算 QPS（每秒请求数）

    return 0;
}
