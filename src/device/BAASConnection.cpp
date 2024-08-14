//
// Created by pc on 2024/7/22.
//

#include "device/BAASConnection.h"
#include "device/BAASAdbUtils.h"
#include "BAASImageResource.h"

using namespace std;
BAASConnection::BAASConnection(BAASUserConfig *cfg) : BAASConnectionAttr(cfg) {
    detect_device();
    adb_connect();
    detect_package();
    set_server();
    set_activity();
    set_language();

    resource->load(server, language);

    check_mumu_app_keep_alive();
}

BAASConnection::BAASConnection(const std::string &cfg_path) : BAASConnectionAttr(cfg_path) {
    detect_device();
    adb_connect();
    detect_package();
    set_server();
    set_activity();
    set_language();

    resource->load(server, language);

    check_mumu_app_keep_alive();
}

void BAASConnection::detect_device() {
    logger->hr("Detect Device");

    vector<pair<string, int>> d;
    int n_available;
    vector<std::string> unavailable, available;

    for(int j = 1; j <= 2; ++j) {
        n_available = 0;
        unavailable.clear();
        available.clear();

        BAASAdbClient::list_device(d);

        // Show available devices
        logger->BAASInfo("Available devices are listed below, choose the one you want to run BAAS on.");
        for(auto &i: d) {
            if(i.second == 1) {
                logger->BAASInfo(to_string(++n_available) + " : [ "+ i.first + " ]");
                available.push_back(i.first);
            }
            else unavailable.push_back(i.first);
        }
        if(n_available == 0) logger->BAASInfo("No available device. Please check your device connection.");

        // Show unavailable devices
        if(!unavailable.empty()) {
            logger->BAASWarn("Detected but unavailable devices are listed below.");
            for(int i = 0; i<unavailable.size(); ++i) {
                logger->BAASWarn(to_string(i+1) + " : [ " + unavailable[i] + " ]");
            }
        }

        // brute force connect
        if(serial == "auto" && n_available == 0) {
            logger->BAASWarn("No available device found.");
            brute_force_connect(d);
        }
        else break;
    }

    // auto device detection
    if(serial == "auto") {
        if (n_available == 0) {
            logger->BAASCritical("No available device found, please set the exact serial in the config [ /emulator/serial ].");
            throw RequestHumanTakeOver("No available device found.");
        } else if (n_available == 1) {
            logger->BAASInfo("Auto device detection found only one device, using it");
            serial = d[0].first;
            config->replace("/emulator/serial", serial);
            config->save();
        } else if(n_available == 2 && (available[0] == "127.0.0.1:7555" && available[1] == "127.0.0.1:16384") || (available[1] == "127.0.0.1:7555" && available[0] == "127.0.0.1:16384")) {
            logger->BAASInfo("Find Same MuMu12 Device, using it");
            serial = "127.0.0.1:16384";
            config->replace("/emulator/serial", serial);
            config->save();
        }
        else{
            logger->BAASCritical("Multiple devices found, choose the one you want to run and fill in the config [ /emulator/serial ].");
            throw RequestHumanTakeOver("Multiple devices detected.");
        }
    }

    pair<string, string> p = port_emu_pair_serial(serial);
    // is a ldplayer, choose the current online device, LDPlayer serial jumps between `127.0.0.1:5555+{X}` and `emulator-5554+{X}`
    if(!(p.first.empty() ||p.second.empty())) {
        pair<string, int> port_device = {"", -1}, emu_device = {"", -1};
        for(auto &i: d) {
            if(i.first == p.first) port_device = i;
            if(i.first == p.second) emu_device = i;
        }
        // 127.0.0.1:5555 online and emulator-5554 offline
        if(port_device.second == 1 && emu_device.second == 0) {
            logger->BAASInfo("Find LDPlayer device serial pair [ " + port_device.first + " ] and [ " + emu_device.first + " ]");
            logger->BAASInfo("Using the online one : port_device [ " + port_device.first + " ]");
            serial = port_device.first;
        }
        // 127.0.0.1:5555 offline and emulator-5554 online
        else if(port_device.second == 0 && emu_device.second == 1) {
            logger->BAASInfo("Find LDPlayer device serial pair [ " + port_device.first + " ] and [ " + emu_device.first + " ]");
            logger->BAASInfo("Using the online one : emu_device [ " + emu_device.first + " ]");
            serial = emu_device.first;
        }
        else {
            // try to find the ld serial, if fail try paired serial
            bool found = false;
            for(auto &i: available) {
                if(i == serial) {
                    logger->BAASInfo("LDPlayer device serial [ " + serial + " ] is online.");
                    found = true;
                    break;
                }
            }
            if(!found) {
                logger->BAASInfo("Current serial [ " + serial + " ] is not online, try to find it's twin serial.");
                if (port_device.second == 1) {
                    logger->BAASInfo("Twin port_serial [ " + port_device.first + " ] is online, using it.");
                    serial = port_device.first;
                } else if (emu_device.second == 1) {
                    logger->BAASInfo("Twin emu_serial [ " + emu_device.first + " ] is online, using it.");
                    serial = emu_device.first;
                }
            }
        }
    }
    // redirect 127.0.0.1:7555 --> 127.0.0.1:16xxx
    if(serial == "127.0.0.1:7555") {
        vector<string> may_mumu_d;
        for(int i = 1; i <= 2; ++i) {
            for(auto &j: available)
                if(BAASUtil::isMuMuFamily(j)) may_mumu_d.push_back(j);
            if(may_mumu_d.size() == 1) {
                logger->BAASWarn("Redirect MuMu12 from [ " + serial + " ] to [ " + may_mumu_d[0] + " ]");
                serial = may_mumu_d[0];
                config->replace("/emulator/serial", serial);
                config->save();
            }
            else if(may_mumu_d.size() >= 2) {
                logger->BAASWarn("Multiple MuMu12 devices found, cannot redirect.");
                break;
            }
            else{
                if(is_mumu_over_version_356()) {
                    logger->BAASWarn("Device [ + " + serial + " [ is MuMu12 but corresponding port not found.");
                    brute_force_connect(d);
                    available.clear();
                    logger->BAASInfo("Available devices.");
                    for(auto &j: d)
                        if(j.second == 1) {
                            available.push_back(j.first);
                            logger->BAASInfo(j.first);
                        }
                    if(available.empty()) logger->BAASInfo("No available device.");
                }
            }
        }
    }

    // if 16384 is occupied, try 16385
    if(is_mumu12_family()) {
        bool matched = false;
        int device_port, this_port;
        this_port = BAASUtil::serial2port(serial);
        for(auto &i: available) {
            device_port = BAASUtil::serial2port(i);
            if(device_port == this_port) {
                matched = true;
                break;
            }
        }
        if(!matched) {
            logger->BAASWarn("MuMu12 device [ " + serial + " ] not online, search port near by.");
            int diff;
            for(auto &i: available) {
                device_port = BAASUtil::serial2port(i);
                diff = abs(device_port - this_port);
                if(diff <= 2) {
                    logger->BAASInfo("Assume MuMu12 device serial switch from [ " + serial + " ] to [ " + i + " ]");
                    serial = i;
                    break;
                }
            }
        }
    }
}

void BAASConnection::adb_connect() {
    vector<pair<string, int>> d;
    BAASAdbClient::list_device(d);
    for(auto &i: d) {
        switch (i.second) {
            case 0:
                logger->BAASWarn("Device [ " + i.first + " ] is offline, disconnect it before connecting.");
                break;
            case 1:
                logger->BAASInfo("Find Device [ " + i.first + " ] online.");
                break;
            case 2:
                logger->BAASWarn("Device [ " + i.first + " ] is unauthorized, please accept ADB debugging on your device.");
                break;
            default:
                logger->BAASWarn("Device [ " + i.first + " ] is having a unknown status.");
                break;
        }
    }
    if(serial.find("emulator-")!=std::string::npos) {
        logger->BAASInfo(serial + " is an emulator-* serial, skip adb connect.");
        return;
    }
    else {
        if(BAASUtil::re_match(serial, R"([a-zA-Z0-9]+$)")) {
            logger->BAASInfo("Serial [ " + serial + " seems to be a Android serial, skip adb connect");
            return;
        }
    }

    std::string msg;
    for(int i = 1; i<=3; ++i) {
        msg = adb.connect(serial);
        logger->BAASInfo(msg);
        if (msg.find("connected") != std::string::npos) {
            // Connected to 127.0.0.1:59865
            // Already connected to 127.0.0.1:59865
            return;
        } else if (msg.find("bad port") != std::string::npos) {
            // adb connect 12412341
            // bad port number '12412341'
            logger->BAASError(msg);
            throw RequestHumanTakeOver("Serial Incorrect.");
        } else if (msg.find("10061") != std::string::npos) {
            // No connection could be made because the target machine actively refused it. (10061)
            // cannot connect to 127.0.0.1:134: 由于目标计算机积极拒绝，无法连接。 (10061)
            logger->BAASError(msg);
            logger->BAASError("No such device exists, please restart the emulator or set a correct serial.");
            throw EmulatorNotRunningError("No such device exists.");
        }
    }

    logger->BAASWarn("Failed to connect [ " + serial + " after 3 trial, assume connected.");
    detect_device();
}


void BAASConnection::brute_force_connect(vector<pair<string,int >> &devices) {
    logger->hr("Brute Force Connect");
    BAASAdbClient::list_device(devices);

    for(auto &i: devices)
        if(i.second == 1) adb.connect(i.first);

    BAASAdbClient::list_device(devices);
}

std::pair<std::string, std::string> BAASConnection::port_emu_pair_serial(const string &serial) {
    int port = BAASUtil::serial2port(serial);
    if(serial.starts_with("127.0.0.1:") && port >= 5555 && port <= 5555 + 32)
        return {"127.0.0.1:" + to_string(port), "emulator-" + to_string(port-1)};
    else if(serial.starts_with("emulator-") && port >= 5554 && port <= 5554 + 32)
        return {"127.0.0.1:" + to_string(port+1), "emulator-" + to_string(port)};
    return {"", ""};
}


std::string BAASConnection::adb_shell_bytes(const string &command) {
    BAASAdbDevice d = BAASAdbDevice(&adb, serial);
    string res;
    d.shellBytes(command, res);
    if(res.ends_with("\n")) res.pop_back();
    return res;
}

std::string BAASConnection::adb_shell_bytes(const vector<std::string> &commandList) {
    BAASAdbDevice d = BAASAdbDevice(&adb, serial);
    string res;
    d.shellBytes(commandList, res);
    return res;
}

std::string BAASConnection::adb_getprop(const string &name) {
    string temp = adb_shell_bytes("getprop " + name);
    return temp;
}

void BAASConnection::check_mumu_app_keep_alive() {
    if(! is_mumu_family()) return;
    string res = nemud_app_keep_alive();
    if(res.empty() || res == "false") return;
    else if(res == "true") {
        logger->BAASCritical("Please close the [ MuMu app_keep_alive ] option in the MuMu settings.");
        throw RequestHumanTakeOver("MuMu app_keep_alive is enabled.");
    }
    else {
        logger->BAASWarn("Unknown nemud.app_keep_alive value : " + res);
        return;
    }
}

std::string BAASConnection::nemud_app_keep_alive() {
    std::string t = adb_getprop("nemud.app_keep_alive");

    logger->BAASInfo("nemud.app_keep_alive : " + t);
    return t;
}


bool BAASConnection::is_avd(const string &serial) {
    pair<string, string> p = BAASConnection::port_emu_pair_serial(serial);
    if(p.first.empty()) return false;

    if(adb_getprop("ro.hardware").find("ranchu") != std::string::npos) {
        return true;
    }
    if(adb_getprop("ro.hardware.audio.primary").find("goldfish") != std::string::npos) {
        return true;
    }
    return false;
}

bool BAASConnection::is_mumu_over_version_356() {
    if(!nemud_app_keep_alive().empty()) return true;
    return false;
}

void BAASConnection::list_package(vector<std::string> &packages) {
    logger->BAASInfo("Get Package List");
    // faster : 10ms
    string res = adb_shell_bytes(R"(pm list packages)");
    BAASUtil::re_find_all(res, R"(package:([^\s]+))", packages);

    if(!packages.empty()) return;
    // slower ,but list system packages : 200 - 500ms
    res = adb_shell_bytes(R"(dumpsys package | grep "Package \[")");
    BAASUtil::re_find_all(res, R"(Package \[([^\]]+)\])", packages);
}

void BAASConnection::list_all_known_packages(vector<std::string> &packages) {
    packages.clear();

    vector<string> valid_packages = static_config->valid_packages();
    assert(!valid_packages.empty());

    vector<string> packages_in_device;
    list_package(packages_in_device);
    for(auto &i: packages_in_device)
        if(std::find(valid_packages.begin(), valid_packages.end(), i) != valid_packages.end())
            packages.push_back(i);
    logger->BAASInfo("Packages Found in device [ " + serial + " ] are listed below.");
    for(int i = 0; i < packages.size(); ++i)logger->BAASInfo(to_string(i+1) + " : [ " + packages[i] + " ]");
}

void BAASConnection::auto_detect_package() {
    logger->BAASInfo("Auto Detect Package");

    vector<string> packages;
    list_all_known_packages(packages);

    if(packages.empty()) {
        logger->BAASCritical("No Blue Archive package found in device [ " + serial + " ], please install game first.");
        throw RequestHumanTakeOver("No valid package found.");
    }
    else if(packages.size() == 1) {
        logger->BAASInfo("Auto package detection found only one package, using it.");
        package_name = packages[0];
        config->replace("/emulator/package_name", package_name);
        config->save();
    }
    else {
        logger->BAASCritical("Multiple packages found, choose the one you want to run and fill in the config [ /emulator/package_name ].");
        throw RequestHumanTakeOver("Multiple packages detected.");
    }
}

int BAASConnection::adb_push(const string &local, const string &remote) {
    BAASAdbDevice d = BAASAdbDevice(&adb, serial);
    return d.push(local, remote);
}

string BAASConnection::adb_command(const string &command) {
    BAASAdbDevice d = BAASAdbDevice(&adb, serial);
    return d.getCommandResult(command);
}

BAASAdbConnection *BAASConnection::adb_shell_stream(const string &command) {
    BAASAdbDevice d = BAASAdbDevice(&adb, serial);
    return d.shellStream(command);
}

BAASAdbConnection *BAASConnection::adb_shell_stream(const vector<std::string> &commandList) {
    BAASAdbDevice d = BAASAdbDevice(&adb, serial);
    return d.shellStream(commandList);
}

BAASAdbConnection *BAASConnection::create_connection(const string &network, const string &address) {
    BAASAdbDevice d = BAASAdbDevice(&adb, serial);
    return d.createConnection(network, address);
}

bool BAASConnection::clear_cache(const string &package) {
    logger->BAASInfo("Clear Cache for package [ " + package + " ]");
    string res = adb_shell_bytes("pm clear " + package);
    if(res.find("Success") != string::npos) return true;
    else {
        logger->BAASError("Clear Cache Failed." + res);
        return false;
    }
}

void BAASConnection::current_app(string &pkg,string &activity, int& pid) {
    logger->BAASInfo("Get Current App");
    string res = adb_shell_bytes("dumpsys activity top");
    vector<smatch> m;
    string pat = R"(ACTIVITY ([^\s]+)/([^/\s]+) \w+ pid=(\d+))";
    BAASUtil::re_find_all(res, pat, m);
    if(m.empty()) {
        logger->BAASError("No current app found.");
        throw RequestHumanTakeOver("Couldn't get focused app.");
    }
    int tar = int(m.size())-1;
    pkg = m[tar][1];
    activity = m[tar][2];
    pid = stoi(m[tar][3]);
}


void BAASConnection::app_stop(const string &package) {
    adb_shell_bytes("am force-stop " + package);
}

void BAASConnection::app_start(const string &package) {
    adb_shell_bytes("monkey -p " + package + " -c android.intent.category.LAUNCHER 1");
}

void BAASConnection::app_start(const string &package, const string &activity) {
    adb_shell_bytes("am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n " + package + "/" + activity);
}

void BAASConnection::start_self() {
    if(activity_name.empty()) app_start(package_name);
    else app_start(package_name, activity_name);
}
BAASAdbDevice *BAASConnection::adb_device() {
    return new BAASAdbDevice(&adb, serial);
}

void BAASConnection::set_server() {
    server = Server::package2server(package_name);
    config->replace("/emulator/server", server);
    logger->BAASInfo("Server : " + server);
}

void BAASConnection::set_language() {
    language = config->getString("/emulator/language", "auto");
    if(language == "auto")
        auto_detect_language();
    logger->BAASInfo("Language : " + language);
}

void BAASConnection::detect_package() {
    package_name = config->getString("/emulator/package_name", "auto");
    if(package_name == "auto")
        auto_detect_package();
    else{
        logger->BAASInfo("Check package [ " + package_name + " ] exist.");
        vector<string> packages;
        list_all_known_packages(packages);
        if(std::find(packages.begin(), packages.end(), package_name) == packages.end()) {
            logger->BAASCritical("Package [ " + package_name + " ] not found in device [ " + serial + " ], please install game first.");
            throw RequestHumanTakeOver("Package not found.");
        }
        logger->BAASInfo("Package Matched");
    }
}

void BAASConnection::set_activity() {
    activity_name = static_config->getString("/activity_name/" + package_name);
    if(activity_name.empty()) {
        logger->BAASInfo("Activity name not found");
    }
    else logger->BAASInfo("Activity name : [ " + activity_name + " ]");
}

void BAASConnection::auto_detect_language() {
    auto languages = static_config->get<vector<string>>("/server_language_choice/" + server);
    assert(!languages.empty());
    if(languages.size() == 1) {
        config->replace("/emulator/language", languages[0]);
        language = languages[0];
    }
    else {
        logger->BAASCritical("Multiple languages found, choose the one you want to run and fill in the config [ /emulator/language ].");
        throw RequestHumanTakeOver("Multiple languages detected.");
    }
}










