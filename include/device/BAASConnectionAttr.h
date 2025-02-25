//
// Created by pc on 2024/7/20.
//

#ifndef BAAS_DEVICE_BAASCONNECTIONATTR_H_
#define BAAS_DEVICE_BAASCONNECTIONATTR_H_

#include <regex>

#include "config.h"

// connection information with config
BAAS_NAMESPACE_BEGIN

class BAASConnectionAttr {
public:
    explicit BAASConnectionAttr(BAASUserConfig *cfg);

    explicit BAASConnectionAttr(const std::string &cfg_path);

    static std::string adb_binary();

    void revise_serial();

    void serial_check();

    inline int port()
    {
        return BAASUtil::serial2port(serial);
    }

    inline bool is_mumu12_family()
    {
        int p = port();
        return p >= 16384 && p <= 17408;
    }

    inline bool is_mumu_family()
    {
        int p = port();
        return (p == 7555 || is_mumu12_family());
    }

    inline bool is_nox_family()
    {
        int p = port();
        return p >= 62001 && p <= 63025;
    }

    inline bool is_wsa()
    {
        std::regex pattern(R"(^wsa)");
        return std::regex_search(serial, pattern);
    }

    inline bool is_emulator()
    {
        return serial.starts_with("emulator-") || serial.starts_with("127.0.0.1:");
    }

    inline bool is_network_device()
    {
        return BAASUtil::re_match(serial, R"(\d+\.\d+\.\d+\.\d+:\d+)");
    }

    inline bool is_over_http() const
    {
        return BAASUtil::re_match(serial, R"(^https?://)");
    }

    inline bool is_chinac_phone_cloud() const
    {
        return BAASUtil::re_match(serial, R"(:30[0-9]$)");
    }

    inline bool is_bluestacks4_hyperv()
    {
        return serial.find("bluestacks4-hyperv") != std::string::npos;
    }

    inline bool is_bluestacks5_hyperv()
    {
        return serial.find("bluestacks5-hyperv") != std::string::npos;
    }

    inline bool is_bluestacks_hyperv()
    {
        return is_bluestacks4_hyperv() || is_bluestacks5_hyperv();
    }

    ~BAASConnectionAttr();

    [[nodiscard]] inline BAASLogger *get_logger() const
    {
        return logger;
    }

    [[nodiscard]] inline std::string get_serial()
    {
        return serial;
    }

    static int LDPlayer_serial2instance_id(const std::string &serial);

protected:
    BAASUserConfig *config;

    BAASLogger *logger;

    std::string serial;

};

BAAS_NAMESPACE_END

#endif //BAAS_DEVICE_BAASCONNECTIONATTR_H_
