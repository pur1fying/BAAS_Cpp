//
// Created by pc on 2025/2/22.
//

#include <utils.h>
#include <ocr/BAASOCR.h>
#include <server.h>
#include "BAASExternalIPC.h"
using namespace baas;
int main()
{
    BAAS_OCR::__init();
    baas::BAASOCR::update_valid_languages();
//    std::vector<std::string> languages({"en-us"});
//    baas_ocr->init(languages);
//    cv::Mat img = cv::imread("test_ocr_for_single_line2.png");
//    baas::TextLine result;
//    baas_ocr->ocr_for_single_line("en-us", img, result, "", nullptr, "");
//    std::cout << result.text << std::endl;
//    baas_ocr->ocr_for_single_line("en-us", img, result, "", nullptr, "I love Aris");
//    std::cout << result.text << std::endl;
//    return 0;
    BAAS_OCR::Server server;

    server.start();
    BAASGlobalLogger->BAASInfo("exit 0");
    return 0;
}