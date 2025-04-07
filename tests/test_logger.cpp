/**
  ******************************************************************************
  * @file           : test_logger.cpp
  * @author         : 18483
  * @brief          : None
  * @attention      : None
  * @date           : 2025/4/5
  ******************************************************************************
  */


#include "../src/include/Krpc_Logger.h"

int main(int argc, char* argv[]) {
    KrpcLogger logger(argv[0]);  // 自动初始化和析构

    KrpcLogger::Info("Service started.");
    KrpcLogger::Warning("This is a warning.");
    KrpcLogger::Error("Something went wrong.");
    // Krpc_Logger::Fatal("Fatal error.");  // 注意：LOG(FATAL) 会终止程序

    return 0;
}

