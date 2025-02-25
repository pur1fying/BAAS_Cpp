//
// Created by pc on 2024/10/8.
//

#ifndef BAAS_OCR_BAASOCR_H_
#define BAAS_OCR_BAASOCR_H_

#include <map>
#include <string>

#include <opencv2/opencv.hpp>
#include "ocr/OcrStruct.h"
#include "ocr/OcrLite.h"
#include "BAASLogger.h"

BAAS_NAMESPACE_BEGIN

class BAASOCR {
public:
    static BAASOCR *get_instance();

    static void update_valid_languages();

    int init(const std::string &language, int gpu_id = -2, int num_thread = 0);

    std::vector<int> init(const std::vector<std::string> &languages, int gpu_id = -2, int num_thread = 0);

    void release_all();

    void test_ocr();

    void ocr(
            const std::string &language,
            const cv::Mat &img,
            OcrResult &result,
            BAASLogger *logger = nullptr,
            const std::string &candidates = std::string());

    void ocr_for_single_line(
            const std::string &language,
            const cv::Mat &img,
            TextLine &result,
            const std::string &log_content = "",
            BAASLogger *logger = nullptr,
            const std::string &candidates = std::string());

    static void ocrResult2json(
            OcrResult &result,
            nlohmann::json &output
    );

    static void string_unique_utf8_characters(
            const std::string &text,
            std::vector<std::string> &output
    );

    static std::string REGEX_UTF8PATTERN;
private:
    BAASOCR();

    static BAASOCR *instance;

    static std::vector<std::string> valid_languages;

    std::map<std::string, OcrLite *> ocr_map;     // language -> ocr

};

extern BAASOCR *baas_ocr;

BAAS_NAMESPACE_END

#endif //BAAS_OCR_BAASOCR_H_
