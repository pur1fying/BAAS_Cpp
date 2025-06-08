//
// Created by pc on 2024/7/20.
//

#include <vector>

#include "config/BAASStaticConfig.h"
#include "device/BAASConnection.h"
#include "device/BAASConnectionAttr.h"

using namespace std;

BAAS_NAMESPACE_BEGIN

BAASConnectionAttr::BAASConnectionAttr(BAASUserConfig *cfg)
{
    logger = cfg->get_logger();
    config = cfg;
    logger->BAASInfo("Adb Binary : " + adb_binary());
    serial = cfg->serial();
    serial_check();
}

BAASConnectionAttr::BAASConnectionAttr(const std::string &cfg_path)
{
    config = new BAASUserConfig(cfg_path);
    logger = config->get_logger();
    serial = config->serial();
}

BAASConnectionAttr::~BAASConnectionAttr()
{

}

std::string BAASConnectionAttr::adb_binary()
{
    // try existing adb.exe
    for (auto t: static_config->adb_binary_dirs()) {
        if (filesystem::exists(t)) {
            return t;
        }
    }
    // try adb in PATH
    return "adb";
}

void BAASConnectionAttr::serial_check()
{
    string old = serial;
    revise_serial();
    if (old != serial) {
        logger->BAASWarn("Serial [ " + old + " ] is revised to [ " + serial + " ]");
        config->update_reference("/emulator/serial", serial);
    }
    logger->BAASInfo("Serial : " + serial);

}

void BAASConnectionAttr::revise_serial()
{
    BAASStringUtil::stringReplace(" ", "", serial);
    BAASStringUtil::stringReplace("。", ".", serial);
    BAASStringUtil::stringReplace("，", ".", serial);
    BAASStringUtil::stringReplace(",", ".", serial);
    BAASStringUtil::stringReplace("：", ":", serial);
    try {
        int port = std::stoi(serial);
        if (port > 1000 && port < 65536) serial = "127.0.0.1" + std::to_string(port);
    } catch (std::exception &e) {}
    if (serial.find("模拟") != std::string::npos) {
        string m;
        BAASStringUtil::re_find(serial, R"(\d+\.\d+\.\d+\.\d+)", m);
        if (!m.empty()) serial = m;
    }
    BAASStringUtil::stringReplace("12127.0.0.1", "127.0.0.1", serial);
    BAASStringUtil::stringReplace("auto127.0.0.1", "127.0.0.1", serial);
}

int BAASConnectionAttr::LDPlayer_serial2instance_id(const string &serial)
{
    pair<string, string> pair_serial = BAASConnection::port_emu_pair_serial(serial);
    int port = serial2port(pair_serial.first);
    if (5555 <= port && port <= 5555 + 32) return int((port - 5555) / 2);
    return -1;
}

BAAS_NAMESPACE_END