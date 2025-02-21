//
// Created by pc on 2024/11/5.
//
#include "string"

#include "nlohmann/json.hpp"

#include "BAASGlobals.h"
#include "config/BAASConfig.h"
#include "config/BAASGlobalSetting.h"

using namespace std;
using json = nlohmann::json;

BAAS_NAMESPACE_BEGIN
BAASGlobalSetting *global_setting = nullptr;

BAASGlobalSetting *BAASGlobalSetting::globalSetting = nullptr;

BAASGlobalSetting *BAASGlobalSetting::getGlobalSetting()
{
    if (globalSetting == nullptr) {
        globalSetting = new BAASGlobalSetting();
    }
    return globalSetting;
}

BAASGlobalSetting::BAASGlobalSetting() : BAASConfig(
        BAAS_CONFIG_DIR / "global_setting.json",
        (BAASLogger *) BAASGlobalLogger
)
{
    create_modify_history_file();
    update_global_setting();
    save();
}

void BAASGlobalSetting::check_global_setting_exist()
{
    std::filesystem::path global_setting_path = BAAS_CONFIG_DIR / "global_setting.json";
    if (!std::filesystem::exists(global_setting_path)) {
        BAASGlobalLogger->sub_title("Write Default Global Setting");
        std::ofstream ofs(BAAS_CONFIG_DIR / "global_setting.json");
        ofs << default_global_setting->get_config().dump(4);
        ofs.close();
    }
    if (!std::filesystem::exists(global_setting_path)) {
        BAASGlobalLogger->BAASError("Failed to write default global setting.");
        throw PathError("Global Setting Do Not Exist");
    }
}


void BAASGlobalSetting::update_global_setting()
{
    logger->BAASInfo("Global config update");
    my_flatten();
    default_global_setting->my_flatten();

    vector<pair<string, json>> keys;
    // remove not exist key
    json default_config = default_global_setting->get_config();
    for (auto &it: config.items())
        if (!default_config.contains(it.key())) {
            logger->BAASInfo("Remove [ " + it.key() + " ]");
            keys.push_back({it.key(), {}});
        }
    for (auto &it: keys) removeByKey(it.first);

    // insert new key
    keys.clear();
    for (auto &it: default_config.items())
        if (!config.contains(it.key())) {
            logger->BAASInfo("Add [ " + it.key() + " ]");
            keys.emplace_back(it.key(), it.value());
        }
    for (auto &it: keys) updateByKey(it.first, it.second);
    my_unflatten();
    default_global_setting->my_unflatten();
}

void BAASGlobalSetting::create_modify_history_file()
{
    modify_history_path = GlobalLogger::get_folder_path() / "global_setting_change.json";
    std::ofstream modify_record_file(modify_history_path);
    modify_record_file << json::object({}).dump(4);
}

BAAS_NAMESPACE_END