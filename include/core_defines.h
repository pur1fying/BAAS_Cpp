//
// Created by pc on 2025/2/18.
//

#ifndef BAAS_CORE_DEFINES_H_
#define BAAS_CORE_DEFINES_H_

#define BAAS_NAMESPACE_BEGIN namespace baas {
#define BAAS_NAMESPACE_END }

#ifdef _WIN32
    #define UNIX_LIKE_PLATFORM 0
#elif defined(__linux__) || defined(__APPLE__)
    #define UNIX_LIKE_PLATFORM 1
#endif  // _WIN32

#endif // BAAS_CORE_DEFINES_H_
