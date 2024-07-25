//
// Created by pc on 2024/7/22.
//

#ifndef BAAS_DEVICE_BAASCONNECTION_H_
#define BAAS_DEVICE_BAASCONNECTION_H_

#include "BAASConnectionAttr.h"
#include "device/BAASAdbUtils.h"
/*
 *  [ From https://github.com/LmeSzinc/AzurLaneAutoScript/blob/master/module/device/connection.py ]
 */
class BAASConnection : public BAASConnectionAttr {

public:
    explicit BAASConnection(BAASUserConfig* cfg);

    explicit BAASConnection(const std::string& cfg_path);

    // connect to all online devices listed by "adb device" and return serial
    void brute_force_connect(std::vector<std::pair<std::string, int>>& devices);

    // Find available devices If serial=='auto' and only 1 device detected, use it
    void detect_device();

    //    Args:
    //        serial (str):
    //
    //    Returns:
    //        str, str: `127.0.0.1:5555+{X}` and `emulator-5554+{X}`, 0 <= X <= 32
    static std::pair<std::string, std::string> port_emu_pair_serial(const std::string& serial);

    BAASAdbDevice* adb_device();

    std::string adb_shell_bytes(const std::string& command);

    std::string adb_shell_bytes(const std::vector<std::string>& commandList);

    std::string adb_getprop(const std::string& name);

    std::string nemud_app_keep_alive();

    int sdk_ver();

    bool is_avd(const std::string &serial);

    void check_mumu_app_keep_alive();

    bool is_mumu_over_version_356();

    void list_package(std::vector<std::string> &packages);

    // list package appear in static.json "valid_packages"
    void list_all_known_packages(std::vector<std::string> &packages);

    // detect only "package_name" in config is auto
    void auto_detect_package();

    [[nodiscard]] inline const std::string& get_package_name() const{
        return package_name;
    }
private:



    void adb_connect();

    std::string package_name;

};
#endif //BAAS_DEVICE_BAASCONNECTION_H_
