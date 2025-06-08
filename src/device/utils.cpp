//
// Created by pc on 2025/6/8.
//

#include "device/utils.h"

#include <stdexcept>

BAAS_NAMESPACE_BEGIN

void calc_swipe_params(
        int x1,
        int y1,
        int x2,
        int y2,
        double duration,
        int& step_len,
        double& sleep_delay
)
{
    int dis_squared = squared_distance(x1, y1, x2, y2);
    sleep_delay = 0.005;
    if (dis_squared <= 25) step_len = 5;
    else {
        int total_steps = int(duration * 1000) / 5;
        int dis = int(sqrt(dis_squared));
        step_len = int(dis / total_steps) + 1;
        if (step_len < 5) {
            step_len = 5;
            total_steps = dis / 5;
            sleep_delay = duration / total_steps;
        }
    }
}

void insert_swipe(
        std::vector<std::pair<int, int>> &output,
        int start_x,
        int start_y,
        int end_x,
        int end_y,
        int step_len
)
{
    step_len = std::max(1, step_len);

    output.clear();
    output.emplace_back(start_x, start_y);
    int dis = squared_distance(start_x, start_y, end_x, end_y);
    if (dis < step_len * step_len) {
        output.emplace_back(end_x, end_y);
        return;
    }

    double total_len = sqrt(dis);
    int step_num = ceil(total_len / step_len);
    double dx = double((end_x - start_x) * 1.0) / step_num;
    double dy = double((end_y - start_y) * 1.0) / step_num;
    for (int i = 1; i < step_num; i++) {
        output.emplace_back(start_x + int(round(i * dx)), start_y + int(round(i * dy)));
    }

    output.emplace_back(end_x, end_y);
}


int MuMu_serial2instance_id(const std::string &serial)
{
    int port = serial2port(serial);
    port -= 16384;
    int index = port / 32, offset = port % 32;
    if ((offset == 0 || offset == 1 || offset == 2) && index >= 0 && index <= 31) {
        return index;
    }
    return -1;
}


bool serialHost(
        const std::string &serial,
        std::string &host
)
{
    int pos = static_cast<int>(serial.find(':'));
    if (pos == std::string::npos) {
        host = "";
        return false;
    }
    host = serial.substr(0, pos);
    return true;
}

bool serialPort(
        const std::string &serial,
        std::string &port
)
{
    int pos = static_cast<int>(serial.find(':'));
    if (pos == std::string::npos) {
        port = "";
        return false;
    }
    port = serial.substr(pos + 1);
    return true;
}

int serial2port(const std::string &serial)
{
    int res = 0;
    if (serial.starts_with("127.0.0.1:")) {
        try {
            res = stoi(serial.substr(10));
        } catch (std::invalid_argument &e) {}
    } else if (serial.starts_with("emulator-")) {
        try {
            res = stoi(serial.substr(9));
        } catch (std::invalid_argument &e) {}
    }
    return res;
}

bool isMuMuFamily(const std::string &serial)
{
    return serial == "127.0.0.1:7555" or isMuMu12Family(serial);
}

bool isMuMu12Family(const std::string &serial)
{
    int port = serial2port(serial);
    return port >= 16384 and port <= 17408;
}

std::pair<std::string, std::string> serialToHostPort(const std::string &serial)
{
    int pos = int(serial.find(':'));
    if (pos == std::string::npos) {
        return std::make_pair("", "");
    }
    return make_pair(serial.substr(0, pos), serial.substr(pos + 1));
}

BAAS_NAMESPACE_END