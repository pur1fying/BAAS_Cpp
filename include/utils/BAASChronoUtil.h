//
// Created by pc on 2025/6/8.
//

#ifndef BAAS_UTILS_BAASCHRONOUTIL_H_
#define BAAS_UTILS_BAASCHRONOUTIL_H_

#include <thread>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

class BAASChronoUtil {

public:

    static long long getCurrentTimeMS();

    static inline void sleepMS(int ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    static inline void sleepS(int s)
    {
        std::this_thread::sleep_for(std::chrono::seconds(s));
    }

    static int getCurrentTimeStamp();

};


BAAS_NAMESPACE_END

#endif //BAAS_UTILS_BAASCHRONOUTIL_H_
