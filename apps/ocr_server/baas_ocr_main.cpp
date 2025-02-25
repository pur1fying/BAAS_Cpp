//
// Created by pc on 2025/2/22.
//

#include <utils.h>
#include <ocr/BAASOCR.h>
#include <server.h>

using namespace baas;
int main()
{
    BAAS_OCR::__init();

    baas::BAASOCR::update_valid_languages();

    BAAS_OCR::Server server;

    server.start();
    return 0;
}