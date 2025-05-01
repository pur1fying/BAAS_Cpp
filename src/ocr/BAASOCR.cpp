//
// Created by pc on 2024/10/27.
//

#include "ocr/BAASOCR.h"
#include "ocr/OcrUtils.h"
#include "BAASGlobals.h"
#include "config/BAASStaticConfig.h"
#include "config/BAASGlobalSetting.h"

BAAS_NAMESPACE_BEGIN


BAASOCR *BAASOCR::instance = nullptr;

std::vector<std::string> BAASOCR::valid_languages;

std::unique_ptr<ThreadPool> BAASOCR::pool;

std::vector<DbNet*> BAASOCR::uninited_dbnet;

std::vector<AngleNet*> BAASOCR::uninited_anglenet;

std::vector<CrnnNet*> BAASOCR::uninited_crnnnet;

bool BAASOCR::thread_pool_enabled = false;

std::string BAASOCR::REGEX_UTF8PATTERN = R"(([\x00-\x7F]|[\xC0-\xDF][\x80-\xBF]|[\xE0-\xEF][\x80-\xBF]{2}|[\xF0-\xF7][\x80-\xBF]{3}))";

BAASOCR *baas_ocr = nullptr;

BAASOCR *BAASOCR::get_instance()
{
    if (instance == nullptr) {
        instance = new BAASOCR();
    }
    return instance;
}

void BAASOCR::shutdown()
{
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

int BAASOCR::init(
        const std::string &language,
        int gpu_id,
        int num_thread,
        bool enable_cpu_memory_arena
        )
/*
 * 0 : invalid language
 * 1 : success
 * 2 : already inited
 */
{
    auto it = ocr_map.find(language);
    if (it != ocr_map.end()) {
        BAASGlobalLogger->BAASInfo("Ocr [ " + language + " ] already inited");
        return 2;
    }

    if (std::find(valid_languages.begin(), valid_languages.end(), language) == valid_languages.end()) {
        BAASGlobalLogger->BAASError("Invalid language : " + language);
        return 0;
    }

    std::string key = "/ocr_model_name/" + language + "/";
    std::string det = static_config->get(key + "det", std::string());
    std::string cls = static_config->get(key + "cls", std::string());
    std::string rec = static_config->get(key + "rec", std::string());
    std::string keys = static_config->get(key + "dict", std::string());

    auto ocr = new OcrLite();
    BAASGlobalLogger->sub_title("OCR Init");
    BAASGlobalLogger->BAASInfo("language: " + language);
    BAASGlobalLogger->BAASInfo("Det     : " + det);
    BAASGlobalLogger->BAASInfo("Cls     : " + cls);
    BAASGlobalLogger->BAASInfo("Rec     : " + rec);
    BAASGlobalLogger->BAASInfo("Key     : " + keys);


    BAASGlobalLogger->BAASInfo("GPU ID      : " + std::to_string(gpu_id));
    BAASGlobalLogger->BAASInfo("Num Thread  : " + std::to_string(num_thread));
    BAASGlobalLogger->BAASInfo("Memory Pool : " + std::to_string(enable_cpu_memory_arena));

    ocr->get_net(
            det,
            cls,
            rec,
            keys,
            gpu_id,
            num_thread,
            enable_cpu_memory_arena
        );

    ocr_map[language] = ocr;
    return 1;
}

void BAASOCR::get_text_boxes(
        const std::string &language,
        const cv::Mat &img,
        std::vector<TextBox> &result
)
{
    auto res = ocr_map.find(language);
    if (res == ocr_map.end()) {
        BAASGlobalLogger->BAASError("OCR for" + language + " not init");
        return;
    }
    res->second->get_text_boxes(img, result);
}


void BAASOCR::ocr(
        const std::string &language,
        const cv::Mat &img,
        OcrResult &result,
        BAASLogger *logger,
        const std::string &candidates
)
{
    auto res = ocr_map.find(language);
    if (res == ocr_map.end()) {
        if (logger != nullptr) logger->BAASError("OCR for" + language + " not init");
        return;
    }
    std::vector<std::string> unique_candidates;
    string_unique_utf8_characters(candidates, unique_candidates);

    if (logger != nullptr and !candidates.empty()) {
        std::string temp;
        BAASUtil::stringJoin(unique_candidates, "", temp);
        logger->BAASInfo("Ocr Candidates [ " + temp + " ]");
    }
    result = res->second->detect(
            img,
            res->second->padding,
            res->second->maxSideLen,
            res->second->boxScoreThresh,
            res->second->boxThresh,
            res->second->unClipRatio,
            res->second->doAngle,
            res->second->mostAngle,
            unique_candidates
        );

}

void BAASOCR::ocr_for_single_line(
        const std::string &language,
        const cv::Mat &img,
        TextLine &result,
        const std::string &log_content,
        BAASLogger *logger,
        const std::string &candidates
)
{
    auto res = ocr_map.find(language);
    if (res == ocr_map.end()) {
        if (logger != nullptr) logger->BAASError("OCR language [ " + language + " ] not init.");
        return;
    }
    std::vector<std::string> unique_candidates;
    string_unique_utf8_characters(candidates, unique_candidates);
    if (logger != nullptr and !candidates.empty()) {
        std::string temp;
        BAASUtil::stringJoin(unique_candidates, "", temp);
        logger->BAASInfo("Ocr Candidates [ " + temp + " ]");
    }

    res->second->ocr_for_single_line(img, result, unique_candidates);
    if (logger != nullptr) {
        std::string log = "Ocr ";
        if (!log_content.empty()) log += "[ " + log_content + " ] ";
        log += " : " + result.text;
        logger->BAASInfo(log);
        logger->BAASInfo("Time : " + std::to_string(int(result.time)) + "ms");
    }
}

std::vector<int> BAASOCR::init(
        const std::vector<std::string> &languages,
        std::optional<int> gpu_id,
        std::optional<int> num_thread,
        std::optional<bool> enable_cpu_memory_arena
)
{
    // use config in global setting
    if (!gpu_id.has_value())
        gpu_id = global_setting->get("/ocr/gpu_id", -1);
    if (!num_thread.has_value())
        num_thread = global_setting->getInt("/ocr/num_thread", 0);
    if (!enable_cpu_memory_arena.has_value())
        enable_cpu_memory_arena = global_setting->getBool("/ocr/enable_cpu_memory_arena", false);
    std::vector<int> res;
    for (auto &i: languages)
        res.push_back(
                init(
                        i,
                        gpu_id.value(),
                        num_thread.value(),
                        enable_cpu_memory_arena.value()
                    ));

    std::vector<std::future<void>> threads;
    if (thread_pool_enabled) {
        for (auto &i: uninited_dbnet) threads.push_back(pool->submit(OcrLite::submit_dbNet_initModel, i));
        for (auto &i: uninited_anglenet) threads.push_back(pool->submit(OcrLite::submit_angleNet_initModel, i));
        for (auto &i: uninited_crnnnet) threads.push_back(pool->submit(OcrLite::submit_crnnNet_initModel, i));

        for (auto &i: threads) i.get();
        uninited_dbnet.clear();
        uninited_anglenet.clear();
        uninited_crnnnet.clear();
    }
    else {
        for (auto &i: uninited_dbnet) i->initModel();
        for (auto &i: uninited_anglenet) i->initModel();
        for (auto &i: uninited_crnnnet) i->initModel();
        uninited_dbnet.clear();
        uninited_anglenet.clear();
        uninited_crnnnet.clear();
    }
    return res;
}

void BAASOCR::test_ocr()
{
    BAASGlobalLogger->hr("Test OCR");
    std::filesystem::path path = BAAS_IMAGE_RESOURCE_DIR / "test_images";
    std::filesystem::path temp;
    TextLine result;
    for (auto &i: ocr_map) {
        temp = path / (i.first + ".png");
        if (!std::filesystem::exists(temp)) {
            BAASGlobalLogger->BAASError("File not found : ");
            BAASGlobalLogger->Path(temp, 3);
            continue;
        }
        cv::Mat img = cv::imread(temp.string());
        ocr_for_single_line(i.first, img, result, i.first, (BAASLogger *) (BAASGlobalLogger));
    }
}

void BAASOCR::ocrResult2json(
        OcrResult &result,
        nlohmann::json &output,
        uint8_t options
)
{
    output.clear();
    output["dbNet_time"] = result.dbNetTime;
    output["full_detection_time"] = result.detectTime;
    output["str_res"] = result.strRes;
    output["text_list"] = nlohmann::json::array();
    nlohmann::json text;
    for (auto &textBlock: result.textBlocks) {
        if (textBlock.text.empty())
            continue;
        text["text"] = textBlock.text;
        // angle net info
        if (options & 0b001) {
            text["angle_net"]["index"] = textBlock.angleIndex;
            text["angle_net"]["score"] = textBlock.angleScore;
            text["angle_net"]["time"] = std::round(textBlock.angleTime);
        }
        // character score
        if (options & 0b010) {
            text["char_scores"] = textBlock.charScores;
        }
        // position
        if (options & 0b100) {
            text["position"] = nlohmann::json::array();
            text["position"].push_back({textBlock.boxPoint[0].x, textBlock.boxPoint[0].y});
            text["position"].push_back({textBlock.boxPoint[1].x, textBlock.boxPoint[1].y});
            text["position"].push_back({textBlock.boxPoint[2].x, textBlock.boxPoint[2].y});
            text["position"].push_back({textBlock.boxPoint[3].x, textBlock.boxPoint[3].y});
        }
        text["crnn_time"] = std::round(textBlock.crnnTime);
        output["text_list"].push_back(text);
    }
}

void BAASOCR::string_unique_utf8_characters(
        const std::string &text,
        std::vector<std::string> &output
)
{
    if (text.empty()) return;
    output.clear();
    std::vector<std::string> matches;
    BAASUtil::re_find_all(text, REGEX_UTF8PATTERN, matches);
    for (auto &i: matches) output.push_back(i);
    sort(output.begin(), output.end());
    std::set<std::string> unique(output.begin(), output.end());
    output.assign(unique.begin(), unique.end());
}

BAASOCR::BAASOCR()
{
}

BAASOCR::~BAASOCR()
{
    /*
       1. release all models 
       2. shut down thread pool 
       note : if we do not shut down thread pool, operating system will call terminate , process can't exit with code 0 (-6 instead) 
    */ 
    release_all();
    disable_thread_pool();
}

void BAASOCR::update_valid_languages()
{
    BAASGlobalLogger->sub_title("OCR Valid Languages");
    std::string language;
    auto j = static_config->get<nlohmann::json>("/ocr_model_name", nlohmann::json());
    for (auto &i: j.items()) {
        language = i.key();
        BAASGlobalLogger->BAASInfo(language);
        valid_languages.push_back(i.key());
    }
}

bool BAASOCR::is_valid_language(const std::string &language)
{
    for (auto& _lang : valid_languages) if(language == _lang) return true;
    return false;
}

void BAASOCR::release_all()
{
    BAASGlobalLogger->sub_title("OCR Lite Release All");
    for (auto &i: ocr_map) {
        i.second->dbNet.reset();
        i.second->angleNet.reset();
        i.second->crnnNet.reset();
        delete i.second;
    }
    DbNet::try_release_all();
    AngleNet::try_release_all();
    CrnnNet::try_release_all();
    ocr_map.clear();
}

bool BAASOCR::release(const std::string &language)
{
    auto it = ocr_map.find(language);
    if (it == ocr_map.end()) {
        BAASGlobalLogger->BAASError("Language " + language + " not exist.");
        return false;
    }
    BAASGlobalLogger->sub_title("OCR Release : " + language);
    std::string key = it->second->dbNet->getModelPath().string();
    it->second->dbNet.reset();
    DbNet::try_release_net(key);

    key = it->second->angleNet->getModelPath().string();
    it->second->angleNet.reset();
    AngleNet::try_release_net(key);

    key = it->second->crnnNet->get_joined_path().string();
    it->second->crnnNet.reset();
    CrnnNet::try_release_net(key);

    ocr_map.erase(it);
    return true;
}

std::vector<bool> BAASOCR::release(const std::vector<std::string> &languages)
{
    std::vector<bool> res;
    for (auto &i: languages) res.push_back(release(i));
    return res;
}

bool BAASOCR::language_inited(const std::string &language)
{
    return ocr_map.find(language) != ocr_map.end();
}

void BAASOCR::enable_thread_pool(unsigned int thread_count)
{
    if (pool != nullptr) {
        pool->shutdown();
        pool.reset();
    }
    pool = std::make_unique<ThreadPool>(int(thread_count));
    pool->init();
    thread_pool_enabled = true;
}

void BAASOCR::disable_thread_pool()
{
    thread_pool_enabled = false;
    if (pool != nullptr) {
        pool->shutdown();
        pool.reset();
    }
}




BAAS_NAMESPACE_END
