//
// Created by pc on 2024/4/12.
//

#ifndef BAAS_BAASLOGGER_H_
#define BAAS_BAASLOGGER_H_
#include <mutex>
#include <set>
#include <map>

#include <iostream>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

extern std::string BAAS_LOGGER_HR_LINE;

void gen_hr_msg(const std::string &msg,std::string& out);

class GlobalLogger {
public:
    static GlobalLogger* getGlobalLogger();
    inline void BAASTrance(const std::string& message) {
        if(enable & 0b1) consoleLogger->trace(message);
        if(enable & 0b10)fileLogger->trace(message);
    }

    inline void BAASTrance(const std::vector<std::string>& messages) {
        for (auto &message : messages) {
            if(enable & 0b1)consoleLogger->trace(message);
            if(enable & 0b10)fileLogger->trace(message);
        }
    }

    inline void BAASDebug(const std::string& message) {
        if(enable & 0b1) consoleLogger->debug(message);
        if(enable & 0b10) fileLogger->debug(message);
    }

    inline void BAASDebug(const std::vector<std::string>& messages) {
        for (auto &message : messages) {
            if(enable & 0b1)  consoleLogger->debug(message);
            if(enable & 0b10)fileLogger->debug(message);
        }
    }

    inline void BAASWarn(const std::string& message) {
        if(enable & 0b1) consoleLogger->warn(message);
        if(enable & 0b10) fileLogger->warn(message);
    }

    inline void BAASWarn(const std::vector<std::string>& messages) {
        for (auto &message : messages) {
            if(enable & 0b1) consoleLogger->warn(message);
            if(enable & 0b10) fileLogger->warn(message);
        }
    }

    inline void BAASInfo(const std::string& message) {
        if(enable & 0b1) consoleLogger->info(message);
        if(enable & 0b10) fileLogger->info(message);
    }

    inline void BAASInfo(const std::vector<std::string>& messages) {
        for (auto &message : messages) {
            if(enable & 0b1) consoleLogger->info(message);
            if(enable & 0b10) fileLogger->info(message);
        }
    }

    inline void BAASError(const std::string& message) {
        if(enable & 0b1) consoleLogger->error(message);
        if(enable & 0b10) fileLogger->error(message);
    }

    inline void BAASError(const std::vector<std::string>& messages) {
        for (auto &message : messages) {
            if(enable & 0b1) consoleLogger->error(message);
            if(enable & 0b10) fileLogger->error(message);
        }
    }

    inline void BAASCritical(const std::string& message) {
        if(enable & 0b1) consoleLogger->critical(message);
        if(enable & 0b10) fileLogger->critical(message);
    }

    inline void BAASCritical(const std::vector<std::string>& messages) {
        for (auto &message : messages) {
            if(enable & 0b1) consoleLogger->critical(message);
            if(enable & 0b10) fileLogger->critical(message);
        }
    }
    inline void hr(const std::string& message) {
        std::string msg;
        gen_hr_msg(message, msg);
        if(enable & 0b1) {
            consoleLogger->info(BAAS_LOGGER_HR_LINE);
            consoleLogger->info(msg);
            consoleLogger->info(BAAS_LOGGER_HR_LINE);
        }
        if(enable & 0b10) {
            fileLogger->info(BAAS_LOGGER_HR_LINE);
            fileLogger->info(msg);
            fileLogger->info(BAAS_LOGGER_HR_LINE);
        }
    }

    inline void set_enable(uint8_t enable) {
        this->enable = enable;
    }

    static void clearLogData();

    inline void show_enable() {
        std::cout << "console : " << (enable & 0b1) << " file : " << (enable & 0b10) << std::endl;
    }

    inline static std::string get_folder_path() {
        return folder_path;
    }
private:
    static std::string folder_path;

    static GlobalLogger* global_logger;

    GlobalLogger();

    std::shared_ptr<spdlog::logger> consoleLogger;

    std::shared_ptr<spdlog::logger> fileLogger;

    uint8_t enable;//  0 0 0 0 0 0 0(file) 0(console)

    friend class BAASLogger;
};

extern GlobalLogger* BAASGlobalLogger;

/*
 * each config file has it's unique logger
 */
class BAASLogger {
public:
    static BAASLogger* get(const std::string& name);

    inline void BAASTrance(const std::string &message) {
        if(enable & 0b1) consoleLogger->trace(message);
        if(enable & 0b10)fileLogger->trace(message);
    }

    inline void BAASTrance(const std::vector<std::string> &messages) {
        for (auto &message : messages) {
            if(enable & 0b1) consoleLogger->trace(message);
            if(enable & 0b10) fileLogger->trace(message);
        }
    }

    inline void BAASDebug(const std::string &message) {
        if(enable & 0b1) consoleLogger->debug(message);
        if(enable & 0b10)fileLogger->debug(message);
    }

    inline void BAASDebug(const std::vector<std::string> &messages) {
        for (auto &message : messages) {
            if(enable & 0b1) consoleLogger->debug(message);
            if(enable & 0b10) fileLogger->debug(message);
        }
    }

    inline void BAASWarn(const std::string &message) {
        if(enable & 0b1) consoleLogger->warn(message);
        if(enable & 0b10)fileLogger->warn(message);
    }

    inline void BAASWarn(const std::vector<std::string> &messages) {
        for (auto &message : messages) {
            if(enable & 0b1) consoleLogger->warn(message);
            if(enable & 0b10) fileLogger->warn(message);
        }
    }

    inline void BAASInfo(const std::string &message) {
        if(enable & 0b1) consoleLogger->info(message);
        if(enable & 0b10)fileLogger->info(message);
    }

    inline void BAASInfo(const std::vector<std::string> &messages) {
        for (auto &message : messages) {
            if(enable & 0b1) consoleLogger->info(message);
            if(enable & 0b10) fileLogger->info(message);
        }
    }

    inline void BAASError(const std::string &message) {
        if(enable & 0b1) consoleLogger->error(message);
        if(enable & 0b10)fileLogger->error(message);
    }

    inline void BAASError(const std::vector<std::string> &messages) {
        for (auto &message : messages) {
            if(enable & 0b1) consoleLogger->error(message);
            if(enable & 0b10) fileLogger->error(message);
        }
    }

    inline void BAASCritical(const std::string &message) {
        if(enable & 0b1) consoleLogger->critical(message);
        if(enable & 0b10)fileLogger->critical(message);
    }

    inline void BAASCritical(const std::vector<std::string> &messages) {
        for (auto &message : messages) {
            if(enable & 0b1) consoleLogger->critical(message);
            if(enable & 0b10) fileLogger->critical(message);
        }
    }

    inline void hr(const std::string& message) {
        std::string msg;
        gen_hr_msg(message, msg);
        if(enable & 0b1) {
            consoleLogger->info(BAAS_LOGGER_HR_LINE);
            consoleLogger->info(msg);
            consoleLogger->info(BAAS_LOGGER_HR_LINE);
        }
        if(enable & 0b10) {
            fileLogger->info(BAAS_LOGGER_HR_LINE);
            fileLogger->info(msg);
            fileLogger->info(BAAS_LOGGER_HR_LINE);
        }
    }

    inline void set_enable(uint8_t enable) {
        this->enable = enable;
    }
private:
    explicit BAASLogger(const std::string& name);

    ~BAASLogger();

    uint8_t enable;

    std::string filename;

    std::shared_ptr<spdlog::logger> consoleLogger;

    std::shared_ptr<spdlog::logger> fileLogger;

    static std::map<std::string, BAASLogger*> instances;

};

#endif //BAAS_BAASLOGGER_H_
