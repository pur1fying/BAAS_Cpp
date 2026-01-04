//
// Created by Administrator on 2025/12/21.
//

#ifndef BAAS_BAASEXPORT_H_
#define BAAS_BAASEXPORT_H_

#ifdef _WIN32
    #ifdef BAAS_BUILD_DLL
        #define BAAS_API __declspec(dllexport)
    #else
        #define BAAS_API __declspec(dllimport)
    #endif // BAAS_BUILD_DLL
#endif // _WIN32

#if (UNIX_LIKE_PLATFORM == 1) || defined(__ANDROID__)
    #define BAAS_API __attribute__((visibility("default")))
#endif // (UNIX_LIKE_PLATFORM == 1) || defined(__ANDROID__)

#endif //BAAS_BAASEXPORT_H_
