//
// Created by pc on 2024/4/12.
//


#include "BAASLogger.h"

#include <iostream>

#include "BAASUtil.h"
#include "BAASGlobals.h"

using namespace std;

string GlobalLogger::folder_path;

GlobalLogger *GlobalLogger::global_logger = nullptr;

GlobalLogger *GlobalLogger::getGlobalLogger() {
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

GlobalLogger::GlobalLogger() {
    try{
        enable = 0b11;
        consoleLogger = spdlog::stdout_color_mt("console");
        if(!filesystem::exists("output")) {
            filesystem::create_directory("output");
        }

        string currTime = BAASUtil::getCurrentTimeString();
        folder_path = BAAS_OUTPUT_DIR + "\\" + currTime;
        filesystem::create_directory(folder_path);
        fstream file(folder_path + "\\global_log.txt", ios::out);
        file.close();
        fileLogger = spdlog::basic_logger_mt("file_logger", folder_path + "\\global_log.txt");
    }
    catch (const spdlog::spdlog_ex& ex) {
        cout << "Log init failed: " << ex.what() << endl;
    }
    spdlog::set_default_logger(consoleLogger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    consoleLogger->set_level(spdlog::level::debug);
    fileLogger->set_level(spdlog::level::debug);
}



void GlobalLogger::clearLogData() {
    for(filesystem::directory_iterator itr(BAAS_OUTPUT_DIR); itr != filesystem::directory_iterator(); ++itr) {
        if((itr->path().string() != folder_path) && filesystem::is_directory(itr->path())) {
            BAASGlobalLogger->BAASInfo("Remove folder : [ " + itr->path().string() + " ]." );
            filesystem::remove_all(itr->path());
        }
    }
}



GlobalLogger *BAASGlobalLogger = nullptr;

map<string, BAASLogger*> BAASLogger::instances;

BAASLogger *BAASLogger::get(const std::string& name) {
    auto it = instances.find(name);
    if(it != instances.end()) return it->second;

    instances[name] = new BAASLogger(name);
    return instances[name];
}

BAASLogger::BAASLogger(const string& name) {
    enable = 0b11;
    consoleLogger = spdlog::stdout_color_mt(name + "_console");
    if(!filesystem::exists("output")) {
        filesystem::create_directory("output");
    }
    string currTime = BAASUtil::getCurrentTimeString();
    filename = name + ".txt";
    fstream file(GlobalLogger::folder_path + "\\" + filename, ios::out);
    file.close();
    fileLogger = spdlog::basic_logger_mt(name + "_file_logger", GlobalLogger::folder_path + "\\" + filename);
}



BAASLogger::~BAASLogger() = default;
