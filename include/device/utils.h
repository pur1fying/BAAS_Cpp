//
// Created by pc on 2025/6/8.
//

#ifndef BAAS_DEVICE_UTILS_H_
#define BAAS_DEVICE_UTILS_H_

#include "core_defines.h"

#include <vector>
#include <string>

BAAS_NAMESPACE_BEGIN

// not use real distance for accuracy and int will not overflow in this case (x and y are both less than 10000)
static inline int squared_distance(
        int x1,
        int y1,
        int x2,
        int y2
)
{
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

void insert_swipe(
        std::vector<std::pair<int, int>> &output,
        int start_x,
        int start_y,
        int end_x,
        int end_y,
        int step_len
);

void calc_swipe_params(
        int x1,
        int y1,
        int x2,
        int y2,
        double duration,
        int &step_len,
        double &sleep_delay
);

int MuMu_serial2instance_id(const std::string& serial);

bool serialHost(
        const std::string &serial,
        std::string &host
);

bool serialPort(
        const std::string &serial,
        std::string &port
);

int serial2port(const std::string &serial);

bool isMuMuFamily(const std::string &serial);

bool isMuMu12Family(const std::string &serial);

std::pair<std::string, std::string> serialToHostPort(const std::string &serial);

BAAS_NAMESPACE_END

#endif //BAAS_DEVICE_UTILS_H_
