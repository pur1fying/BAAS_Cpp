//
// Created by pc on 2024/10/27.
//

#include "ocr/BAASOCR.h"
#include "config/BAASStaticConfig.h"
#include "config/BAASGlobalSetting.h"
#include "ocr/OcrUtils.h"
#include "BAASGlobals.h"

BAASOCR* BAASOCR::instance = nullptr;

BAASOCR* baas_ocr = nullptr;

BAASOCR *BAASOCR::get_instance() {
    if (instance == nullptr) {
        instance = new BAASOCR();
    }
    return instance;
}

bool BAASOCR::init(const std::string &language) {
    auto it = ocr_map.find(language);
    if (it != ocr_map.end()) {
        BAASGlobalLogger->BAASInfo("Ocr " + language + " already inited");
        return true;
    }
    std::string key = "/ocr_model_name/" + language + "/";
    std::string det = static_config->get(key + "det", std::string());
    std::string cls = static_config->get(key + "cls", std::string());
    std::string rec = static_config->get(key + "rec", std::string());
    std::string keys = static_config->get(key + "dict", std::string());

    auto ocr = new OcrLite();
    ocr->initLogger(true, false, false);
    BAASGlobalLogger->BAASInfo("Ocr init lan [ " + language + " ]");
    BAASGlobalLogger->BAASInfo("Det : " + det);
    BAASGlobalLogger->BAASInfo("Cls : " + cls);
    BAASGlobalLogger->BAASInfo("Rec : " + rec);
    BAASGlobalLogger->BAASInfo("Key : " + keys);
    ocr->get_net(det, cls, rec, keys);
    ocr->initModels();
    ocr_map[language] = ocr;
    return true;
}

void BAASOCR::ocr(const std::string &language, const cv::Mat &img, OcrResult &result,
                  const std::string &log_content, BAASLogger *logger) {
    auto res = ocr_map.find(language);
    if (res == ocr_map.end()) {
        if(logger != nullptr) logger->BAASError("OCR for" + language + " not init");
        return;
    }
    result = res->second->detect(img, res->second->padding, res->second->maxSideLen,
                                 res->second->boxScoreThresh, res->second->boxThresh, res->second->unClipRatio,
                                 res->second->doAngle, res->second->mostAngle);
}

void BAASOCR::ocr_for_single_line(const std::string &language, const cv::Mat &img, TextLine &result,
                                const std::string& log_content, BAASLogger *logger) {
    auto res = ocr_map.find(language);
    if (res == ocr_map.end()) {
        if(logger != nullptr) logger->BAASError("OCR for" + language + " not init");
        return;
    }
    res->second->ocr_for_single_line(img, result);
    if(logger != nullptr) {
        logger->BAASInfo("Ocr [ " + log_content + " ] : " + result.text);
        logger->BAASInfo("Time : " + std::to_string(int(result.time)) + "ms");
    }
}

std::vector<bool> BAASOCR::init(const std::vector<std::string> &languages) {
    std::vector<bool> res;
    for (auto &i: languages) {
        res.push_back(init(i));
    }
    return res;
}

void BAASOCR::test_ocr() {
    BAASGlobalLogger->hr("Test OCR");
    std::string path = BAAS_IMAGE_RESOURCE_DIR + "\\test_ocr";
    std::string temp;
    TextLine result;
    for(auto &i: ocr_map) {
        temp = path + "\\" + i.first + ".png";
        if (!std::filesystem::exists(temp)) {
            BAASGlobalLogger->BAASError("File not found : " + temp);
            continue;
        }
        cv::Mat img = cv::imread(temp);
        ocr_for_single_line(i.first, img, result, i.first, (BAASLogger*)(BAASGlobalLogger));
    }
}


