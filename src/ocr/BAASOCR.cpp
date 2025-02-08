//
// Created by pc on 2024/10/27.
//

#include "ocr/BAASOCR.h"
#include "config/BAASStaticConfig.h"
#include "config/BAASGlobalSetting.h"
#include "ocr/OcrUtils.h"
#include "BAASGlobals.h"

BAASOCR* BAASOCR::instance = nullptr;

std::string BAASOCR::REGEX_UTF8PATTERN =  R"(([\x00-\x7F]|[\xC0-\xDF][\x80-\xBF]|[\xE0-\xEF][\x80-\xBF]{2}|[\xF0-\xF7][\x80-\xBF]{3}))";

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

void BAASOCR::ocr(const std::string &language, const cv::Mat &img, OcrResult &result, BAASLogger *logger,
                    const std::string &candidates) {
    auto res = ocr_map.find(language);
    if (res == ocr_map.end()) {
        if(logger != nullptr) logger->BAASError("OCR for " + language + " not init");
        return;
    }
    std::string candidates_cpy = candidates;
    std::vector<std::string> unique_candidates;
    string_unique_utf8_characters(candidates_cpy, unique_candidates);

    if(logger != nullptr and !candidates.empty()) {
        std::string temp;
        BAASUtil::stringJoin(unique_candidates, "", temp);
        logger->BAASInfo("Ocr Candidates [ " + temp + " ]");
    }
    result = res->second->detect(img, res->second->padding, res->second->maxSideLen,
                                 res->second->boxScoreThresh, res->second->boxThresh, res->second->unClipRatio,
                                 res->second->doAngle, res->second->mostAngle, unique_candidates);

}

void BAASOCR::ocr_for_single_line(const std::string &language, const cv::Mat &img, TextLine &result,
                                const std::string& log_content, BAASLogger *logger, const std::string &candidates) {
    auto res = ocr_map.find(language);
    if (res == ocr_map.end()) {
        if(logger != nullptr) logger->BAASError("OCR for " + language + " not init");
        return;
    }
    std::string candidates_cpy = candidates;
    std::vector<std::string> unique_candidates;
    string_unique_utf8_characters(candidates_cpy, unique_candidates);
    if(logger != nullptr and !candidates.empty()) {
        std::string temp;
        BAASUtil::stringJoin(unique_candidates, "", temp);
        logger->BAASInfo("Ocr Candidates [ " + temp + " ]");
    }

    res->second->ocr_for_single_line(img, result, unique_candidates);
    if(logger != nullptr) {
        std::string log = "Ocr ";
        if (!log_content.empty()) log += "[ " + log_content + " ] ";
        log += " : " + result.text;
        logger->BAASInfo(log);
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

void BAASOCR::ocrResult2json(OcrResult &result, nlohmann::json &output) {
    output.clear();
    output["dbNet_time"] = result.dbNetTime;
    output["full_detection_time"] = result.detectTime;
    output["str_res"] = result.strRes;
    output["text_list"] = nlohmann::json::array();
    nlohmann::json text;
    int min_x, min_y, max_x, max_y;
    for (auto &textBlock: result.textBlocks) {
        if (textBlock.text.empty()) continue;
        text["text"] = textBlock.text;
        text["position"] = nlohmann::json::array();
        min_x = result.boxImg.cols;
        min_y = result.boxImg.rows;
        max_x = 0;
        max_y = 0;
        for (auto &point: textBlock.boxPoint) {
            min_x = std::min(min_x, point.x);
            min_y = std::min(min_y, point.y);
            max_x = std::max(max_x, point.x);
            max_y = std::max(max_y, point.y);
        }
        text["position"].push_back({min_x, min_y});
        text["position"].push_back({max_x, max_y});
        text["/angle_net/index"_json_pointer] = textBlock.angleIndex;
        text["/angle_net/score"_json_pointer] = textBlock.angleScore;
        text["/angle_net/time"_json_pointer] = textBlock.angleTime;
        text["char_scores"] = textBlock.charScores;
        text["crnn_time"] = textBlock.crnnTime;
        output["text_list"].push_back(text);
    }
}

void BAASOCR::string_unique_utf8_characters(const std::string &text, std::vector<std::string> &output) {
    if (text.empty()) return;
    output.clear();
    std::vector<std::string> matches;
    BAASUtil::re_find_all(text, REGEX_UTF8PATTERN, matches);
    for (auto &i: matches) output.push_back(i);
    sort(output.begin(), output.end());
    std::set<std::string> unique(output.begin(), output.end());
    output.assign(unique.begin(), unique.end());
}


