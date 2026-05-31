//
// Created by pc on 2025/6/8.
//

#ifndef BAAS_UTILS_BAASSYSTEMUTILS_H_
#define BAAS_UTILS_BAASSYSTEMUTILS_H_

#include "core_defines.h"

#include <cstdio>
#include <string>
#include <vector>

BAAS_NAMESPACE_BEGIN

class BAASSystemUtil {

public:

    /*
     * Initialize winsock to use socket functions
     */
    static bool initWinsock();

    static std::string getStreamOutput(std::FILE *stream);

    static void executeCommandWithoutOutPut(const std::string &command);

    static void executeCommandWithoutOutPut(
            const std::vector<std::string> &commandList,
            int n
    );

    static std::string executeCommandAndGetOutput(const std::string &command);

    static std::string executeCommandAndGetOutput(
            const std::vector<std::string> &commandList,
            int n
    );

    static std::FILE* executeCommand(const std::string &command);


};

BAAS_NAMESPACE_END

#endif //BAAS_UTILS_BAASSYSTEMUTILS_H_
