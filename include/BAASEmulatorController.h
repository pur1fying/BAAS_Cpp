//
// Created by pc on 2024/4/11.
//

#ifndef BAAS_BAASEMULATORCONTROLLER_H_
#define BAAS_BAASEMULATORCONTROLLER_H_
#include <map>
#include <vector>

#include <opencv2/opencv.hpp>

#include "BAASUtil.h"
#include "BAASGlobals.h"
#include "BAASLogger.h"
class ConnectionAttr {

};

struct AutoScanPort {
    explicit AutoScanPort() = default;
};
class Device {
public:
    ~Device();
    cv::Mat screenshot();
    std::string serial,path;
    std::pair<int, int> resolution;
private:
    Device(std::string serial);
    Device(std::string serial,std::string path);
    void initDeviceInfo();
    friend class BAASEmulatorController;
};
class BAASEmulatorController {
public:
    static BAASEmulatorController* getInstance();

    static void disconnectAllEmulators();

    static Device* connect(AutoScanPort);

    static Device* connect(const std::string serial);

    static void listControlledEmulators();

    static void startEmulator(const std::string &emulatorName);

    static void stopEmulator(const std::string &emulatorName);

    static std::vector<std::string> detectEmulators();

    static void startAdbServer();

    static void stopAdbServer();

private:
    BAASEmulatorController();
    static BAASEmulatorController* instance;
    static std::vector<Device*> deviceList;
};
#endif //BAAS_BAASEMULATORCONTROLLER_H_
