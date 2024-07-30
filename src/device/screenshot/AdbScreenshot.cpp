//
// Created by pc on 2024/7/27.
//

#include "device/screenshot/AdbScreenshot.h"


using namespace std;

AdbScreenshot::AdbScreenshot(BAASConnection *connection) : BaseScreenshot(connection) {

}

void AdbScreenshot::init() {
    logger->hr("Screenshot Method AdbScreenshot");
    string ret = connection->adb_shell_bytes("echo 000");
    assert(ret == "000\n");
}

void AdbScreenshot::screenshot(cv::Mat &img) {
    string ret = connection->adb_shell_bytes("screencap -p");
    if(ret.size() < 500)
        logger->BAASWarn("Unexpected screenshot size : " + to_string(ret.size()));
    vector<uchar> data(ret.begin(), ret.end());
    img = cv::imdecode(data, cv::IMREAD_COLOR);
}

void AdbScreenshot::exit() {
    logger->BAASInfo("Screenshot Method AdbScreenshot Exit");

}

bool AdbScreenshot::is_lossy() {
    return false;
}
