#include "BAASEmulatorController.h"
#include <string>
#include <vector>
#include <mutex>
using namespace std;
using namespace cv;
BAASEmulatorController* BAASEmulatorController::instance = nullptr;
vector<Device*> BAASEmulatorController::deviceList = vector<Device*>();

Device::Device(string serial) {
    this->serial = serial;
}

Device::Device(string serial, string path) {
    this->serial = serial;
    this->path = path;
}

Mat Device::screenshot() {
    /*
     *  method in consideration :
     *  1. adb exec-out screencap -p
     *  2. win-api desktop duplication (window must be visible)
     *  3. mumu emulator api
     *  4. obs method
     */
    FILE *fp = BAASUtil::executeCommand("adb exec-out screencap -p");

    int startTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    std::vector<unsigned char> buffer;
    const int chunk_size = 4096;
    while (!feof(fp)) {
        unsigned char chunk[chunk_size];
        size_t bytesRead = fread(chunk, 1, chunk_size, fp);
        if (bytesRead > 0) {
            buffer.insert(buffer.end(), chunk, chunk + bytesRead);
        }
    }
    _pclose(fp);
    int endTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    BAASLoggerInstance->BAASInfo("Screenshot Time : " + to_string(endTime - startTime) + "ms");
    cv::Mat image = cv::imdecode(buffer, cv::IMREAD_COLOR);
    return image;
}

vector<string> BAASEmulatorController::detectEmulators() {
    BAASLoggerInstance->BAASInfo("-- BAAS Detect Emulators --");
    string output = BAASUtil::executeCommandAndGetOutput("adb devices");
    vector<string> emulators;
    size_t pos = 0;
    while((pos = output.find("\n")) != string::npos) {
        string line = output.substr(0, pos);
        if(line.find("List of devices attached") != string::npos) {
            output = output.substr(pos + 1);
            continue;
        }
        if(line.find("device") != string::npos) {
            line = line.substr(0, line.find("device") - 1);
            emulators.push_back(line);
        }
        output = output.substr(pos + 1);
    }
    if(emulators.empty()) BAASLoggerInstance->BAASInfo("NO EMULATOR CONNECTED");
    else {

        BAASLoggerInstance->BAASInfo("Emulators Connected : ");
        for(auto &emulator : emulators) BAASLoggerInstance->BAASInfo(emulator);
    }
    return emulators;
}

BAASEmulatorController* BAASEmulatorController::getInstance() {
    if(instance == nullptr) {
        mutex m;
        m.lock();
        if(instance == nullptr) {
            instance = new BAASEmulatorController();
        }
        m.unlock();
    }
    return instance;
}

void BAASEmulatorController::startAdbServer() {
    BAASLoggerInstance->BAASInfo("-- BAAS Start adb server --");
    BAASUtil::executeCommandWithoutOutPut("adb start-server");
}
void BAASEmulatorController::stopAdbServer() {
    BAASLoggerInstance->BAASInfo("-- BAAS Stop adb server --");
    BAASUtil::executeCommandWithoutOutPut("adb kill-server");
}

BAASEmulatorController::BAASEmulatorController() {

}

Device::~Device() {

}

void BAASEmulatorController::disconnectAllEmulators() {
    BAASLoggerInstance->BAASInfo("-- BAAS Disconnect all emulators --");
    string output = BAASUtil::executeCommandAndGetOutput("adb disconnect");
    BAASLoggerInstance->BAASInfo("adb : " + output);
}

Device* BAASEmulatorController::connect(string serial) {
    BAASLoggerInstance->BAASInfo("-- BAAS Connect to Emulator : " + serial + " --");
    for(int i = 0; i < deviceList.size(); i++) {
        if(deviceList[i]->serial == serial) {
            BAASLoggerInstance->BAASInfo("Instance exists");
            return deviceList[i];
        }
    }
    string output = BAASUtil::executeCommandAndGetOutput("adb connect " + serial);
    BAASLoggerInstance->BAASInfo("adb : " + output);
    Device device(serial);
    return &device;
}

void BAASEmulatorController::listControlledEmulators() {
    BAASLoggerInstance->BAASInfo("-- BAAS List Controlled Emulators --");
    if(deviceList.empty()) {
        BAASLoggerInstance->BAASInfo("NO EMULATOR CONTROLLED");
    } else {
        for(auto &device : deviceList) {
            BAASLoggerInstance->BAASInfo("Emulator : " + device->serial);
        }
    }
}