// Created by pc on 2024/8/9.
//
#include "device/control/BAASControl.h"

using namespace std;

vector<string> BAASControl::available_methods;

BAASControl::BAASControl(const std::string& method, double screen_ratio, BAASConnection *connection) {
    assert(connection != nullptr);
    this->connection = connection;
    logger = connection->get_logger();

    available_methods = static_config->get<std::vector<std::string>>("available_control_methods");
    logger->BAASInfo("Available control methods : ");
    logger->BAASInfo(available_methods);


    this->ratio = screen_ratio;

    control = nullptr;
    set_control_method(method);
}

void BAASControl::init() {
    control->init();
}

void BAASControl::click(BAASPoint point, uint8_t type, int offset, const string &description) {
    click(point.x, point.y, type, offset, description);
}

void BAASControl::click(int x, int y, uint8_t type, int offset,const std::string &description) {
    set_x_y_offset(x, y, type, offset);
    x = int(double(x) * 1.0 * ratio);
    y = int(double(y) * 1.0 * ratio);
    string msg = "Click (" + to_string(x) + "\t, " + to_string(y) + "\t) ";
    if(!description.empty())msg += description;
    logger->BAASInfo(msg);
    control->click(x, y);
}

void BAASControl::click(BAASPoint point, int count, uint8_t type, int offset, double interval, double pre_wait, double post_wait, const string &description) {
    click(point.x, point.y, count, type, offset, interval, pre_wait, post_wait, description);
}

void BAASControl::click(int x, int y, int count, uint8_t type, int offset, double interval, double pre_wait,double post_wait, const string &description) {
    string msg = "Click (\t" + to_string(x) + ",\t" + to_string(y) + " ) ";
    set_x_y_offset(x, y, type, offset);
    x = int(double(x) * 1.0 * ratio);
    y = int(double(y) * 1.0 * ratio);
    if(count > 1)msg += " " + to_string(count) + " times ";
    if(!description.empty())msg += description;
    if(pre_wait > 0) BAASUtil::sleepMS(int(pre_wait * 1000));

    int itv = int(interval * 1000);
    for(int i = 0; i < count; i++) {
        control->click(x, y);
        if(i < count - 1) BAASUtil::sleepMS(itv);
    }
    if(post_wait > 0) BAASUtil::sleepMS(int(post_wait * 1000));
    logger->BAASInfo(msg);
}

void BAASControl::long_click(BAASPoint point, double duration, uint8_t type, int offset) {
    long_click(point.x, point.y, duration, type, offset);
}

void BAASControl::long_click(int x, int y, double duration, uint8_t type, int offset) {
    set_x_y_offset(x, y, type, offset);
    x = int(double(x) * 1.0 * ratio);
    y = int(double(y) * 1.0 * ratio);
    string msg = "Long Click (" + to_string(x) + "\t, " + to_string(y) + "\t) ";
    logger->BAASInfo(msg);
    control->long_click(x, y, duration);
}

void BAASControl::swipe(BAASPoint start, BAASPoint end, double duration) {
    swipe(start.x, start.y, end.x, end.y, duration);
}

void BAASControl::swipe(int x1, int y1, int x2, int y2, double duration) {
    x1 = int(double(x1) * 1.0 * ratio);
    y1 = int(double(y1) * 1.0 * ratio);
    x2 = int(double(x2) * 1.0 * ratio);
    y2 = int(double(y2) * 1.0 * ratio);
    string msg = "Swipe from (" + to_string(x1) + "\t, " + to_string(y1) + "\t) --> (" + to_string(x2) + "\t, " + to_string(y2) + "\t), duration : " + to_string(duration);
    logger->BAASInfo(msg);
    control->swipe(x1, y1, x2, y2, duration);
}

void BAASControl::set_control_method(const std::string& method, bool exit) {
    if(std::find(available_methods.begin(), available_methods.end(), method) == available_methods.end()) {
        logger->BAASCritical("Unsupported control method : [ " + method + " ]");
        throw RequestHumanTakeOver("Unsupported control method: " + method);
    }

    if(method == control_method) {
        logger->BAASInfo("Control method is already set to " + method);
        return;
    }

    if(control != nullptr) {
        if(exit) {
            logger->BAASInfo("Exiting current control method : [ " + control_method + " ]");
            control->exit();
        }
        delete control;
    }

    control_method = method;
    if(control_method == "adb") {
        control = new AdbControl(connection);
    } else if(control_method == "scrcpy") {
        control = new ScrcpyControl(connection);
    } else if(control_method == "nemu") {
        control = new NemuControl(connection);
    }
    init();
}

void BAASControl::set_x_y_offset(int &x, int &y, uint8_t type, int size) {
    switch (type) {
        case OFFSET_TYPE_NOCHANGE:
            break;
        case OFFSET_TYPE_RECTANGLE:{
            x += BAASUtil::genRandInt(-size, size);
            y += BAASUtil::genRandInt(-size, size);
            break;
        }
        case OFFSET_TYPE_CIRCLE:{
            double angle = BAASUtil::genRandDouble(0, 2 * M_PI);
            x += int(size * cos(angle));
            y += int(size * sin(angle));
            break;
        }
        default:
            logger->BAASError("Invalid offset type : " + to_string(type));
            break;
    }
}

void BAASControl::exit() {
    control->exit();
}













