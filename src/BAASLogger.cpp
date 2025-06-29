//
// Created by pc on 2024/4/12.
//
#include "BAASLogger.h"

#include <fstream>
#include <spdlog/async.h>

#include "BAASGlobals.h"
#include "BAASExceptions.h"

using namespace std;

BAAS_NAMESPACE_BEGIN

std::filesystem::path GlobalLogger::folder_path;

GlobalLogger* GlobalLogger::global_logger = nullptr;

const string BAAS_LOGGER_HR_LINE = std::string(80, '-');

const string BAAS_LOGGER_DIVIDER_LINE = std::string(80, '=');

std::string GlobalLogger::current_time_string()
{
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm localTime = *std::localtime(&timeT);
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d_%H.%M.%S");
    std::string formattedTime = oss.str();
    CURRENT_TIME_STRING = oss.str();
    return CURRENT_TIME_STRING;
}

void gen_hr_msg(
        const string& msg,
        string& out
)
{
    int msg_len = int(msg.length());
    int left_space_len = (80 - 2 - msg_len) / 2;    // space len / 2
    int right_space_len = 80 - msg_len - left_space_len - 2;
    out = "|" + std::string(left_space_len, ' ') + msg + std::string(right_space_len, ' ') + "|";
}


GlobalLogger* GlobalLogger::getGlobalLogger()
{
    if (global_logger == nullptr) {
        mutex m;
        m.lock();
        if (global_logger == nullptr) {
            global_logger = new GlobalLogger();
        }
        m.unlock();
    }
    return global_logger;
}

GlobalLogger::GlobalLogger()
{
    try {
        enable = 0b11;
#ifndef __EMSCRIPTEN__
        spdlog::init_thread_pool(8192, 1);
#endif // __EMSCRIPTEN__
        consoleLogger = spdlog::stdout_color_mt("console");
        if (!filesystem::exists(BAAS_OUTPUT_DIR)) filesystem::create_directory(BAAS_OUTPUT_DIR);
        string currTime = current_time_string();
        folder_path = BAAS_OUTPUT_DIR / currTime;
        filesystem::create_directory(folder_path);
        fstream file(folder_path / "global_log.txt", ios::out);
        // iterate current dir and print all file recursive
        std::filesystem::path current_path = filesystem::current_path();
        file.close();
        fileLogger = spdlog::basic_logger_mt("file_logger", (folder_path / "global_log.txt").string());
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::string msg = "Logger Init Failed : " + std::string(ex.what());
        throw RuntimeError(msg);
    }
    spdlog::set_default_logger(consoleLogger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    consoleLogger->set_level(spdlog::level::debug);
    fileLogger->set_level(spdlog::level::debug);
}


void GlobalLogger::clearLogData()
{
    for (filesystem::directory_iterator itr(BAAS_OUTPUT_DIR); itr != filesystem::directory_iterator(); ++itr) {
        if ((itr->path().string() != folder_path) && filesystem::is_directory(itr->path())) {
            BAASGlobalLogger->BAASInfo("Remove folder : [ " + itr->path().string() + " ].");
            filesystem::remove_all(itr->path());
        }
    }
}

GlobalLogger::GlobalLogger(const int)
{

}

GlobalLogger::~GlobalLogger()
{
    flush();
    fileLogger.reset();
    consoleLogger.reset();
    BAASGlobalLogger = nullptr;
}

GlobalLogger* BAASGlobalLogger = nullptr;

map<string, BAASLogger*> BAASLogger::instances;

BAASLogger* BAASLogger::get(const std::string& name)
{
    auto it = instances.find(name);
    if (it != instances.end()) return it->second;

    instances[name] = new BAASLogger(name);
    return instances[name];
}

BAASLogger::BAASLogger(const string& name) : GlobalLogger(1)
{
    enable = 0b11;
    consoleLogger = spdlog::stdout_color_mt(name + "_console");
    if (!filesystem::exists("output")) {
        filesystem::create_directory("output");
    }
    string currTime = current_time_string();
    filename = name + ".txt";
    fstream file(GlobalLogger::folder_path / filename, ios::out);
    file.close();
    fileLogger = spdlog::basic_logger_mt<spdlog::async_factory>(
                                                name + "_file_logger", 
                                                (GlobalLogger::folder_path / filename).string(),
                                                true
                                            );
}


BAASLogger::~BAASLogger() = default;

BAAS_NAMESPACE_END