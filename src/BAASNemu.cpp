//
// Created by pc on 2024/5/22.
//

#include "BAASNemu.h"
using namespace std;

BAASNemu *BAASNemu::instance = nullptr;
BAASNemu::BAASNemu() {
    cout<<nemuDllPath;
    cout<<std::filesystem::exists(nemuDllPath);
    hDllInst = LoadLibrary(nemuDllPath.c_str());
    if (hDllInst == NULL) {
        std::cout << "LoadLibrary failed" << std::endl;
        throw std::runtime_error("LoadLibrary : [ " + nemuDllPath + " ] failed");
    }
    nemu_connect = (nemuConnect)GetProcAddress(hDllInst, "nemu_connect");
    nemu_disconnect = (nemuDisconnect)GetProcAddress(hDllInst, "nemu_disconnect");
    nemu_capture_display = (nemuCaptureDisplay)GetProcAddress(hDllInst, "nemu_capture_display");
    nemu_input_text = (nemuInputText)GetProcAddress(hDllInst, "nemu_input_text");
    nemu_input_event_touch_down = (nemuInputEventTouchDown)GetProcAddress(hDllInst, "nemu_input_event_touch_down");
    nemu_input_event_touch_up = (nemuInputEventTouchUp)GetProcAddress(hDllInst, "nemu_input_event_touch_up");
    nemu_input_event_key_down = (nemuInputEventKeyDown)GetProcAddress(hDllInst, "nemu_input_event_key_down");
    nemu_input_event_key_up = (nemuInputEventKeyUp)GetProcAddress(hDllInst, "nemu_input_event_key_up");
}

BAASNemu *BAASNemu::getInstance() {
    if (instance == nullptr) instance = new BAASNemu();
    return instance;
}

bool BAASNemu::connect(const string installPath,const string serial) {
    int displayId = BAASUtil::MuMuSerialToDisplayId(serial);
    if(displayId == -1) {
        BAASLoggerInstance->BAASError("Invalid serial");
        return false;
    }
    int connection = nemu_connect(BAASUtil::stringToWString(installPath).c_str(), displayId);
    if (connection == 0) {
        BAASLoggerInstance->BAASError("Nemu connect failed");
        return false;
    }
    MuMuDevice *device = new MuMuDevice();
    device->installPath = installPath;
    device->displayId = displayId;
    device->connectionId = connection;
    connections[connection] = device;
    if(!getResolution(connection, displayId, device->resolution)) {
        BAASLoggerInstance->BAASError("Nemu get display resolution failed.");
        return false;
    }
    device->pixels.resize(device->resolution.first * device->resolution.second * 4);
    vector<string> msg = {
            "Nemu connected :   {",
            "                   Serial:\t" + serial,
            "                   MuMuPath:\t" + installPath,
            "                   DisplayID:\t" + to_string(displayId),
            "                   ConnID:\t" + to_string(connection),
            "                   Resolution:\t" + to_string(device->resolution.first) + "x" + to_string(device->resolution.second),
            "                   }"
    };
    BAASLoggerInstance->BAASInfo(msg);
    return true;
}

bool BAASNemu::disconnect(const int connectionID) {
    if(connections.find(connectionID) == connections.end()) {
        BAASLoggerInstance->BAASError("Invalid connectionID");
        return false;
    }
    MuMuDevice *device = connections[connectionID];
    nemu_disconnect(connectionID);
    connections.erase(connectionID);
    delete device;
    return true;
}

bool BAASNemu::screenshot(const int connectionID, cv::Mat &image) {
    MuMuDevice *device = connections[connectionID];
    if(nemu_capture_display(connectionID, 0, int(device->pixels.size()), &device->resolution.first, &device->resolution.second, device->pixels.data()) != 0) {
        return false;
    }
    device->imageOpMutex.lock();
    image = cv::Mat(device->resolution.second, device->resolution.first, CV_8UC4, device->pixels.data());
    cv::cvtColor(image, image, cv::COLOR_BGRA2RGB);
    cv::flip(image, image, 0);
    device->imageOpMutex.unlock();
    return true;
}


bool BAASNemu::getResolution(int connectionId, int displayId, std::pair<int, int> &resolution) {
    if (nemu_capture_display(connectionId, displayId, 0, &resolution.first, &resolution.second, nullptr) != 0) {
        BAASLoggerInstance->BAASError("Nemu get resolution failed");
        return false;
    }
    return true;
}