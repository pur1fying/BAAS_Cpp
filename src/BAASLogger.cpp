//
// Created by pc on 2024/4/12.
//


#include "BAASLogger.h"
#include <iostream>
using namespace std;
BAASLogger *BAASLogger::instance = nullptr;
BAASLogger *BAASLogger::getInstance() {
    if (instance == nullptr) {
        mutex m;
        m.lock();
        if (instance == nullptr) {
            instance = new BAASLogger();
        }
        m.unlock();
    }
    return instance;
}

BAASLogger::BAASLogger() {
    consoleLogger = spdlog::stdout_color_mt("console");
    fileLogger = spdlog::basic_logger_mt("file_logger", "logs.txt");
    spdlog::set_default_logger(consoleLogger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    consoleLogger->set_level(spdlog::level::debug);
    fileLogger->set_level(spdlog::level::debug);
}

void BAASLogger::BAASTrance(std::string message) {
    consoleLogger->trace(message);
    fileLogger->trace(message);
}

void BAASLogger::BAASTrance(std::vector<std::string> messages) {
    for (auto &message : messages) {
        consoleLogger->trace(message);
        fileLogger->trace(message);
    }
}

void BAASLogger::BAASDebug(std::string message) {
    consoleLogger->debug(message);
    fileLogger->debug(message);
}

void BAASLogger::BAASDebug(std::vector<std::string> messages) {
    for (auto &message : messages) {
        consoleLogger->debug(message);
        fileLogger->debug(message);
    }
}

void BAASLogger::BAASWarn(std::string message) {
    consoleLogger->warn(message);
    fileLogger->warn(message);
}

void BAASLogger::BAASWarn(std::vector<std::string> messages) {
    for (auto &message : messages) {
        consoleLogger->warn(message);
        fileLogger->warn(message);
    }
}

void BAASLogger::BAASInfo(std::string message) {
    consoleLogger->info(message);
    fileLogger->info(message);
}

void BAASLogger::BAASInfo(std::vector<std::string> messages) {
    for (auto &message : messages) {
        consoleLogger->info(message);
        fileLogger->info(message);
    }
}

void BAASLogger::BAASError(std::string message) {
    consoleLogger->error(message);
    fileLogger->error(message);
}

void BAASLogger::BAASError(std::vector<std::string> messages) {
    for (auto &message : messages) {
        consoleLogger->error(message);
        fileLogger->error(message);
    }
}

void BAASLogger::BAASCritical(std::string message) {
    consoleLogger->critical(message);
    fileLogger->critical(message);
}

void BAASLogger::BAASCritical(std::vector<std::string> messages) {
    for (auto &message : messages) {
        consoleLogger->critical(message);
        fileLogger->critical(message);
    }
}


BAASLogger *BAASLoggerInstance = nullptr;



