//
// Created by pc on 2024/4/12.
//
#ifndef BAAS_UTILS_BAASUTIL_H_
#define BAAS_UTILS_BAASUTIL_H_



#include <chrono>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <random>
#include <regex>
#include <thread>

#include "core_defines.h"
#include "BAASStringUtil.h"

BAAS_NAMESPACE_BEGIN

class BAASUtil {

public:

    static double genRandDouble(
            const double &min,
            const double &max
    );

    static int genRandInt(
            const int &min,
            const int &max
    );

    static bool endsWith(
            const std::string &src,
            const std::string &suffix
    );

    // from tinyObjLoader
    // See
    // http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
    static std::istream &safeGetLine(
            std::istream &is,
            std::string &t
    );

};

BAAS_NAMESPACE_END

#endif //BAAS_UTILS_BAASUTIL_H_
