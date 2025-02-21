//
// Created by pc on 2024/7/18.
//

#include "config/BAASUserConfig.h"

using namespace std;
using json = nlohmann::json;

BAAS_NAMESPACE_BEGIN

BAASUserConfig *config_template = nullptr;

void BAASUserConfig::config_update()
{
    logger->BAASInfo("CONFIG KEY UPDATE");
    my_flatten();
    config_template->my_flatten();

    vector<pair<string, json>> keys;
    // remove not exist key
    for (auto &it: config.items())
        if (!config_template->config
                            .contains(it.key())) {
            logger->BAASInfo("Remove [ " + it.key() + " ]");
            keys.push_back({it.key(), {}});
        }
    for (auto &it: keys) removeByKey(it.first);

    // insert new key
    keys.clear();
    for (auto &it: config_template->config
                                  .items())
        if (!config.contains(it.key())) {
            logger->BAASInfo("Add [ " + it.key() + " ]");
            keys.emplace_back(it.key(), it.value());
        }
    for (auto &it: keys) updateByKey(it.first, it.second);
    my_unflatten();
    config_template->my_unflatten();
}

void BAASUserConfig::update_name()
{
    logger->BAASInfo("CONFIG NAME UPDATE");
    my_flatten();
    string t;
    for (auto &it: config_name_change->get_config()
                                           .items()) {
        assert(it.value()
                 .is_string());
        t = it.value();
        if (config.contains(it.key())) {
            updateByKey(it.value(), config[it.key()]);
            removeByKey(it.key());
        } else {
            if (!config.contains(t)) {
                logger->BAASWarn("Didn't find either new( " + t + " ) or old( " + it.key() + " ) key in config :");
                logger->Path(path, 3);
            }
        }
    }
    my_unflatten();
}

BAAS_NAMESPACE_END
