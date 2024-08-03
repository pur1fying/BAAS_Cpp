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

    int adb_push(const std::string& local, const std::string& remote);

    std::string adb_command(const std::string& command);

    std::string adb_shell_bytes(const std::string& command);

    std::string adb_shell_bytes(const std::vector<std::string>& commandList);

    BAASAdbConnection* adb_shell_stream(const std::string& command);

    BAASAdbConnection* adb_shell_stream(const std::vector<std::string>& commandList);

    BAASAdbConnection* create_connection(const std::string& network, const std::string& address);

    bool clear_cache(const std::string& package);

    void current_app(std::string& pkg, std::string& activity, int& pid);

    void app_stop(const std::string& package);

    void app_start(const std::string& package);

    void app_start(const std::string& package, const std::string& activity);

    void start_self();

    std::string adb_getprop(const std::string& name);

    std::string nemud_app_keep_alive();

    BAASAdbDevice* adb_device();

    inline int sdk_ver() {
        std::string t = adb_getprop("ro.build.version.sdk");
        logger->BAASInfo("SDK Version : " + t);
        try{
            return stoi(t);
        }catch (std::invalid_argument& e) {
            logger->BAASWarn("Invalid SDK Version : " + t);
            return 0;
        }
    }

    inline std::string cpu_abi() {
        std::string res = adb_getprop("ro.product.cpu.abi");
        if(res.empty())logger->BAASError("Invalid CPU ABI : " + res);
        if(res[res.size()-1] == '\n') res.pop_back();
        return res;
    }

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

    [[nodiscard]] inline const std::string& get_server() const{
        return server;
    }

    [[nodiscard]] inline const std::string& get_language() const{
        return language;
    }

    [[nodiscard]] inline const std::string emulator_folder_path() const{
        return config->getString("/emulator/emulator_folder_path");
    }

    void detect_package();

    void set_activity();

    void set_server();

    void set_language();

    void auto_detect_language();


private:

    void adb_connect();

    std::string package_name;

    std::string activity_name;

    std::string server;

    std::string language;

};
#endif //BAAS_DEVICE_BAASCONNECTION_H_
