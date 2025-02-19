//
// Created by pc on 2024/4/30.
//

#ifndef BAAS_NETWORK_H_
#define BAAS_NETWORK_H_

#include <string>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

class Network {
public:
    static const std::string TCP;
    static const std::string UNIX;
    static const std::string DEV;
    static const std::string LOCAL;
    static const std::string LOCAL_RESERVED;
    static const std::string LOCAL_FILESYSTEM;
    static const std::string LOCAL_ABSTRACT;
};

BAAS_NAMESPACE_END

#endif //BAAS_NETWORK_H_
