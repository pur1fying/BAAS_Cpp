//
// Created by pc on 2025/6/8.
//

#include "utils/BAASChronoUtil.h"

#include <chrono>

BAAS_NAMESPACE_BEGIN

long long BAASChronoUtil::getCurrentTimeMS()
{
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count();
}

int BAASChronoUtil::getCurrentTimeStamp()
{
   return static_cast<int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

BAAS_NAMESPACE_END
