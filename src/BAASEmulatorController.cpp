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
    BAASGlobalLogger->BAASInfo("Screenshot Time : " + to_string(endTime - startTime) + "ms");
    cv::Mat image = cv::imdecode(buffer, cv::IMREAD_COLOR);
    return image;
}

vector<string> BAASEmulatorController::detectEmulators() {
    BAASGlobalLogger->BAASInfo("-- BAAS Detect Emulators --");
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
    if(emulators.empty()) BAASGlobalLogger->BAASInfo("NO EMULATOR CONNECTED");
    else {

        BAASGlobalLogger->BAASInfo("Emulators Connected : ");
        for(auto &emulator : emulators) BAASGlobalLogger->BAASInfo(emulator);
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
    BAASGlobalLogger->BAASInfo("Start adb server.");
    BAASUtil::executeCommandWithoutOutPut("adb start-server");
}
void BAASEmulatorController::stopAdbServer() {
    BAASGlobalLogger->BAASInfo("Stop adb server.");
    BAASUtil::executeCommandWithoutOutPut("adb kill-server");
}

BAASEmulatorController::BAASEmulatorController() {

}

Device::~Device() {

}

void BAASEmulatorController::disconnectAllEmulators() {
    BAASGlobalLogger->BAASInfo("-- BAAS Disconnect all emulators --");
    string output = BAASUtil::executeCommandAndGetOutput("adb disconnect");
    BAASGlobalLogger->BAASInfo("adb : " + output);
}

Device* BAASEmulatorController::connect(string serial) {
    BAASGlobalLogger->BAASInfo("-- BAAS Connect to Emulator : " + serial + " --");
    for(int i = 0; i < deviceList.size(); i++) {
        if(deviceList[i]->serial == serial) {
            BAASGlobalLogger->BAASInfo("Instance exists");
            return deviceList[i];
        }
    }
    string output = BAASUtil::executeCommandAndGetOutput("adb connect " + serial);
    BAASGlobalLogger->BAASInfo("adb : " + output);
    Device device(serial);
    return &device;
}

void BAASEmulatorController::listControlledEmulators() {
    BAASGlobalLogger->BAASInfo("-- BAAS List Controlled Emulators --");
    if(deviceList.empty()) {
        BAASGlobalLogger->BAASInfo("NO EMULATOR CONTROLLED");
    } else {
        for(auto &device : deviceList) {
            BAASGlobalLogger->BAASInfo("Emulator : " + device->serial);
        }
    }
}