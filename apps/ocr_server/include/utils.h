//
// Created by pc on 2025/2/25.
//

#ifndef BAAS_OCR_SERVER_CPP_UTILS_H_
#define BAAS_OCR_SERVER_CPP_UTILS_H_

#include <definitions.h>
#include <ThreadPool.h>
OCR_NAMESPACE_BEGIN

void __init();

extern ThreadPool *ocr_pool;

OCR_NAMESPACE_END


#endif // BAAS_OCR_SERVER_CPP_UTILS_H_
