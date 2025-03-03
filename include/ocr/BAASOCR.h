//
// Created by pc on 2024/10/8.
//

#ifndef BAAS_OCR_BAASOCR_H_
#define BAAS_OCR_BAASOCR_H_

#include <map>
#include <string>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <ThreadPool.h>

#include "ocr/OcrStruct.h"
#include "ocr/OcrLite.h"
#include "BAASLogger.h"

BAAS_NAMESPACE_BEGIN

class BAASOCR {
public:
    static BAASOCR *get_instance();

    static void enable_thread_pool(unsigned int thread_count = 4);

    static void disable_thread_pool();

    static void update_valid_languages();

    bool language_inited(const std::string &language);

    std::vector<int> init(const std::vector<std::string> &languages, int gpu_id = -2, int num_thread = 0);

    void release_all();

    bool release(const std::string &language);

    std::vector<bool> release(const std::vector<std::string> &languages);

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
            nlohmann::json &output,
            uint8_t options = 0b111
    );

    static void string_unique_utf8_characters(
            const std::string &text,
            std::vector<std::string> &output
    );

    static std::string REGEX_UTF8PATTERN;

private:
    int init(const std::string &language, int gpu_id = -2, int num_thread = 0);

    static std::vector<DbNet*> uninited_dbnet;

    static std::vector<CrnnNet*> uninited_crnnnet;

    static std::vector<AngleNet*> uninited_anglenet;

    static std::unique_ptr<ThreadPool> pool;

    static bool thread_pool_enabled;

    BAASOCR();

    static BAASOCR *instance;

    static std::vector<std::string> valid_languages;

    std::map<std::string, OcrLite*> ocr_map;     // language -> ocr

    friend class OcrLite;
    friend class AngleNet;
    friend class DbNet;
    friend class CrnnNet;
};

extern BAASOCR *baas_ocr;

BAAS_NAMESPACE_END

#endif //BAAS_OCR_BAASOCR_H_
