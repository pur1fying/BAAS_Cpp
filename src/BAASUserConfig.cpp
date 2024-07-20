//
// Created by pc on 2024/7/18.
//

#include "config/BAASUserConfig.h"

using namespace std;

UserConfig *config_template = nullptr;

void UserConfig::serial_check() {
    string old = serial;
    revise_serial(serial);
    if(old != serial) {

    }
}

void UserConfig::config_update() {
    my_flatten();
    for(auto &it : config_template->config.items()) {

    }
}

void UserConfig::revise_serial(string &serial) {
    BAASUtil::stringReplace(" ", "", serial);
    BAASUtil::stringReplace("。", ".", serial);
    BAASUtil::stringReplace("，", ".", serial);
    BAASUtil::stringReplace(",", ".", serial);
    BAASUtil::stringReplace("：", ":", serial);
    try{
        int port = std::stoi(serial);
        if(port > 1000 && port < 65536) serial = "127.0.0.1" + std::to_string(port);
    }catch(std::exception &e) {}
    if(serial.find("模拟") != std::string::npos) {

    }
    BAASUtil::stringReplace("12127.0.0.1", "127.0.0.1", serial);
    BAASUtil::stringReplace("auto127.0.0.1", "127.0.0.1", serial);
}

void UserConfig::update_object() {
    for(auto &it : config_template->config.items()) {
        if(it.value().is_object()) {
            update_object();
        }
    }
}

void UserConfig::update_value() {

}

void UserConfig::update_name() {
    my_flatten();
    string t;
    for(auto &it : config_name_change->get_config().items()) {
        assert(it.value().is_string());
        t = it.value();
        if(config.contains(it.key())) {
            updateByKey(it.value(), config[it.key()]);
            removeByKey(it.key());
        }
        else {
            if(!config.contains(t)) {
                logger->BAASWarn("Didn't find either new( " + t + " ) or old( " + it.key() + " ) key in config file : [ " + path + " ].");
            }
        }
    }
    my_unflatten();
}