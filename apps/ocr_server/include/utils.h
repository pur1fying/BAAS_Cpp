//
// Created by pc on 2025/2/25.
//

#ifndef BAAS_OCR_SERVER_CPP_UTILS_H_
#define BAAS_OCR_SERVER_CPP_UTILS_H_

#include <ThreadPool.h>

#include "definitions.h"
#include "server.h"

OCR_NAMESPACE_BEGIN

extern BAAS_OCR::Server server;

void _init();

void _cleanup();

void server_thread();

void handle_input();

OCR_NAMESPACE_END


#endif // BAAS_OCR_SERVER_CPP_UTILS_H_
