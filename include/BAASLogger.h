//
// Created by pc on 2024/4/12.
//

#ifndef BAAS_CXX_REFACTOR_BAASLOGGER_H
#define BAAS_CXX_REFACTOR_BAASLOGGER_H
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <mutex>

class BAASLogger {
public:
    static BAASLogger* getInstance();

    void BAASTrance(std::string message);

    void BAASTrance(std::vector<std::string>);

    void BAASDebug(std::string message);

    void BAASDebug(std::vector<std::string>);

    void BAASWarn(std::string message);

    void BAASWarn(std::vector<std::string>);

    void BAASInfo(std::string message);

    void BAASInfo(std::vector<std::string>);

    void BAASError(std::string message);

    void BAASError(std::vector<std::string>);

    void BAASCritical(std::string message);

    void BAASCritical(std::vector<std::string>);

private:
    static BAASLogger* instance;
    BAASLogger();
    std::shared_ptr<spdlog::logger> consoleLogger;
    std::shared_ptr<spdlog::logger> fileLogger;
};

extern BAASLogger* BAASLoggerInstance;
#endif //BAAS_CXX_REFACTOR_BAASLOGGER_H
