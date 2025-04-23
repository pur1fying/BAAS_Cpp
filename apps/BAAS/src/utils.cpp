//
// Created by pc on 2025/4/20.
//
#include "utils.h"

#include "config.h"
#include "version.h"
#include "BAASGlobals.h"
#include "ocr/BAASOCR.h"
#include "feature/BAASFeature.h"
#include "procedure/BAASProcedure.h"

#include "module/auto_fight/auto_fight.h"

BAAS_NAMESPACE_BEGIN

void init_globals()
{
    BAASUtil::initWinsock();
    init_path();

    BAASGlobalLogger = GlobalLogger::getGlobalLogger();
    log_git_info();

    BAASGlobalLogger->Path(BAAS_PROJECT_DIR);
    baas_ocr = BAASOCR::get_instance();
    resource = BAASImageResource::get_instance();
    static_config = BAASStaticConfig::getStaticConfig();
    baas_features = BAASFeature::get_instance();
    baas_procedures = BAASProcedure::get_instance();
    GameServer::init();

    config_name_change = new BAASConfig(CONFIG_TYPE_CONFIG_NAME_CHANGE);
    config_template = new BAASUserConfig(CONFIG_TYPE_DEFAULT_CONFIG);
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
}

void register_baas_module(BAAS* baas) {
    baas->register_module_implement_func("AutoFight", baas::AutoFight::implement);
}

BAAS_NAMESPACE_END