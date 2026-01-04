//
// Created by Administrator on 2025/12/21.
//

/*
 *  This file contains functions and variables to export when compiling
 *  BAAS_ocr_server for Android platform
 */

#ifndef BAAS_OCR_SERVER_ANDROID_DEFINITIONS_H_
#define BAAS_OCR_SERVER_ANDROID_DEFINITIONS_H_

#include <BAASExport.h>

#ifdef __cplusplus
extern "C" {
#endif

BAAS_API void start_server(const char* res_dir);

BAAS_API void stop_server();

#ifdef __cplusplus
}
#endif

#endif //BAAS_OCR_SERVER_ANDROID_DEFINITIONS_H_
