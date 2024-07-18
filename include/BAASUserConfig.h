//
// Created by pc on 2024/7/18.
//

#ifndef BAAS_BAASUSERCONFIG
#define BAAS_BAASUSERCONFIG

#include "BAASConfig.h"

class UserConfig : public BAASConfig {
public:
    explicit UserConfig(int config_type) : BAASConfig(config_type) {
        this->serial = getString("/emulator/serial");
    }

    explicit UserConfig(const std::string& path) : BAASConfig(path) {
//        this->serial = std::to_string(getInt("adbPort"));

    }

    void config_update();

    void update_name();
    /*
     * recursively iterate object
     */
    void update_object();

    /*
     * value can be primitive or array
     */
    void update_value();

    static void revise_serial(std::string& serial);

    inline void serial_check();
    inline std::string adbPort() {
        return serial;
    }

private:
    std::string serial;

};

extern UserConfig* config_template;

#endif //BAAS_BAASUSERCONFIG
