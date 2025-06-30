//
// Created by pc on 2025/2/25.
//
#include "utils.h"

#include <config.h>
#include <BAASGlobals.h>
#include <ocr/BAASOCR.h>
#include <BAASExternalIPC.h>

#include "BAAS_OCR_version.h"

using namespace baas;
OCR_NAMESPACE_BEGIN

Server server;

void _init()
{
    init_path();
    BAASGlobalLogger = GlobalLogger::getGlobalLogger();
    BAASGlobalLogger->BAASInfo("BAAS OCR VERSION : " + std::string(BAAS_OCR_VERSION));
    log_git_info();

    BAASGlobalLogger->Path(BAAS_PROJECT_DIR);
    baas_ocr = BAASOCR::get_instance();

    static_config = BAASStaticConfig::getStaticConfig();
    baas::BAASOCR::update_valid_languages();

    default_global_setting = new BAASConfig(CONFIG_TYPE_DEFAULT_GLOBAL_SETTING);

    // check config dir
    BAASGlobalLogger->sub_title("Config Dir");
    BAASGlobalLogger->Path(BAAS_CONFIG_DIR);
    if (!std::filesystem::exists(BAAS_CONFIG_DIR)) {
        BAASGlobalLogger->sub_title("Create Config Dir");
        std::filesystem::create_directory(BAAS_CONFIG_DIR);
    }
    BAASGlobalSetting::check_global_setting_exist();
    global_setting = BAASGlobalSetting::getGlobalSetting();
    spdlog::flush_every(std::chrono::milliseconds(global_setting->getInt("/log/flush_interval", 1000)));
}


void _cleanup()
{
    Shared_Memory::release_all();
    delete default_global_setting;
    delete global_setting;
    delete static_config;
    baas_ocr->shutdown();
    BAASGlobalLogger->sub_title("Logger Shutdown");
    delete BAASGlobalLogger;
    spdlog::shutdown();
}

void server_thread() {
    server.init();
    server.start();
}

void handle_input() {
    std::string input;
    while (true) {
        if (!(std::cin >> input)) { 
            BAASGlobalLogger->BAASInfo("EOF detected. Exiting...");
            break;
        }
        BAASGlobalLogger->BAASInfo("Input : " + input);
        if (input == "exit") {
            break;
        }
        else {
            BAASGlobalLogger->BAASInfo("Invalid Input.");
        }
    }
}

OCR_NAMESPACE_END