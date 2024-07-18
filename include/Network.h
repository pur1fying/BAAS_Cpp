//
// Created by pc on 2024/4/30.
//

#ifndef BAAS_NETWORK_H
#define BAAS_NETWORK_H
#include <string>
class Network{
public:
    static const std::string TCP;
    static const std::string UNIX;
    static const std::string DEV;
    static const std::string LOCAL;
    static const std::string LOCAL_RESERVED;
    static const std::string LOCAL_FILESYSTEM;
    static const std::string LOCAL_ABSTRACT;
};
#endif //BAAS_NETWORK_H
