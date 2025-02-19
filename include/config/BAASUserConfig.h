//
// Created by pc on 2024/7/18.
//

#ifndef BAAS_CONFIG_BAASUSERCONFIG_H_
#define BAAS_CONFIG_BAASUSERCONFIG_H_

#include "config/BAASConfig.h"

BAAS_NAMESPACE_BEGIN

class BAASUserConfig : public BAASConfig {
public:
    explicit BAASUserConfig(int config_type) : BAASConfig(config_type)
    {

    }

    explicit BAASUserConfig(const std::string &path) : BAASConfig(path)
    {

    }

    /*
     * synchronize with default config
     */
    void config_update();

    void update_name();


    inline std::string serial()
    {
        return getString("/emulator/serial");
    }

    inline std::string screenshot_method()
    {
        return getString("/emulator/screenshot_method");
    }

    inline std::string control_method()
    {
        return getString("/emulator/control_method");
    }

    inline const double screenshot_interval()
    {
        return getDouble("/script/screenshot_interval");
    }

    inline bool script_show_image_compare_log()
    {
        return getBool("/script/show_image_compare_log");
    }
};

extern BAASUserConfig *config_template;

BAAS_NAMESPACE_END

#endif //BAAS_CONFIG_BAASUSERCONFIG_H_
