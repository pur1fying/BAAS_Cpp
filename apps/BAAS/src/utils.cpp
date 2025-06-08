//
// Created by pc on 2025/4/20.
//
#include "utils.h"

#include "config.h"
#include "version.h"
#include "BAASGlobals.h"
#include "ocr/BAASOCR.h"
#include "feature/BAASFeature.h"
#include "utils/BAASSystemUtil.h"

#include "module/auto_fight/BAASAutoFight.h"

BAAS_NAMESPACE_BEGIN

std::filesystem::path BAAS_AUTO_FIGHT_WORKFLOW_DIR;

std::filesystem::path BAAS_YOLO_MODEL_DIR;

void init_globals()
{
    BAASSystemUtil::initWinsock();
    init_path();

    BAAS_AUTO_FIGHT_WORKFLOW_DIR = BAAS_RESOURCE_DIR / "auto_fight_workflow";

    BAAS_YOLO_MODEL_DIR = BAAS_RESOURCE_DIR / "yolo_models";

    BAASGlobalLogger = GlobalLogger::getGlobalLogger();
    log_git_info();

    BAASGlobalLogger->Path(BAAS_PROJECT_DIR);
    baas_ocr = BAASOCR::get_instance();
    resource = BAASImageResource::get_instance();
    static_config = BAASStaticConfig::getStaticConfig();
    BAASOCR::update_valid_languages();
    baas_features = BAASFeature::get_instance();

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
    baas->register_module_implement_func("AutoFight", baas::BAASAutoFight::implement);
}

BAAS_NAMESPACE_END