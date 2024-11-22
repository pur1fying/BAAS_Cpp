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

class BAASOCR {
public:
    static BAASOCR* get_instance();

    bool init(const std::string& language);

    std::vector<bool> init(const std::vector<std::string>& languages);

    void test_ocr();

    void ocr(const std::string& language, const cv::Mat& img, OcrResult& result,
                    const std::string& log_content = "",BAASLogger* logger = nullptr);

    void ocr_for_single_line(const std::string& language, const cv::Mat& img, TextLine& result,
                                    const std::string& log_content = "", BAASLogger* logger = nullptr);


private:
    BAASOCR() = default;
    static BAASOCR* instance;
    std::map<std::string, OcrLite*> ocr_map;     // language -> ocr

};

extern BAASOCR* baas_ocr;

#endif //BAAS_OCR_BAASOCR_H_
