//
// Created by pc on 2024/8/13.
//

#include "feature/JudgePointRGBRangeFeature.h"
/*  example
 * "feature_name" : {
 *      "feature_type": 2
 *      "server": "JP",
 *      "language": "ja-jp",
 *      "direction": 1,
 *      "rgb_range": {
 *          "JP_ja-jp": {                       // server_language
 *              "position" : [[x1, y1], [x2, y2]],
 *              "range" : [[r1_min, r1_max, g1_min, g2_max, b1_min, b1_max], [r2_min, r2_max, g2_min, g2_max, b2_min, b2_max]]
 *          }
 *      }
 * }
 */

using namespace std;
using namespace nlohmann;

BAAS_NAMESPACE_BEGIN

bool JudgePointRGBRangeFeature::appear(
        const BAAS *baas,
        BAASConfig &output
)
{
    vector<string> log;
    log.emplace_back("Compare Method : [ JudgePointRGBRangeFeature ].");

    cv::Mat image = baas->latest_screenshot;

    int dir = 1;
    if (image.cols > image.rows) dir = 0;
    log.push_back("Image direction : " + to_string(dir));

    if (dir != feature_direction) {
        log.emplace_back("Screenshot direction not match, Quit.");
        output.insert("log", log);
        return false;
    }

    auto it = rgb_info.find(baas->rgb_feature_key);
    if (it == rgb_info.end()) {
        log.emplace_back("RGB Range not found, Quit.");
        output.insert("log", log);
        return false;
    }

    bool check_around = config->getBool("check_around", false);
    int around_range = config->getInt("around_range", 0);
    for (int i = 0; i < it->second.size(); i++) {
        if (!BAASImageUtil::judge_rgb_range(
                    image,
                    it->second[i].x,
                    it->second[i].y,
                    it->second[i].r_min,
                    it->second[i].r_max,
                    it->second[i].g_min,
                    it->second[i].g_max,
                    it->second[i].b_min,
                    it->second[i].b_max,
                    baas->screen_ratio,
                    check_around,
                    around_range)
        )
        {
            log.emplace_back("Position " + to_string(i) + " not match, Quit.");
            output.insert("log", log);
            return false;
        }
    }

    output.insert("log", log);
    return true;
}

JudgePointRGBRangeFeature::JudgePointRGBRangeFeature(BAASConfig *config) : BaseFeature(config)
{
    json j = config->get_config();
    if (!j.is_object() || !j.contains("rgb_range") || !j["rgb_range"].is_object()) {
        throw JudgePointRGBRangeFeatureError("rgb_range Feature format error");
    }
    vector<RGBInfo> temp_rgb_info_vec;
    for (auto &i: j["rgb_range"].items()) {
        _decode_single_rgb_info(i.value(), temp_rgb_info_vec);
        rgb_info[i.key()] = temp_rgb_info_vec;
    }

    feature_direction = config->getInt("direction", 0);  // 720x1280

}

double JudgePointRGBRangeFeature::self_average_cost(const baas::BAAS *baas)
{
    string server_language = baas->rgb_feature_key;
    auto it = rgb_info.find(server_language);
    return 1.0 * double(it->second.size() * 8);
}

void JudgePointRGBRangeFeature::show()
{
    BAASGlobalLogger->sub_title("JudgePointRGBRangeFeature");
    BAASGlobalLogger->BAASInfo("is_primitive        : [ " + std::to_string(_is_primitive) + " ]");
    BAASGlobalLogger->BAASInfo("and_feature_count   : [ " + std::to_string(and_feature_ptr.size()) + " ]");
    BAASGlobalLogger->BAASInfo("or_feature_count    : [ " + std::to_string(or_feature_ptr.size()) + " ]");
    for (const auto &i: rgb_info) {
        BAASGlobalLogger->BAASInfo("Server Language : " + i.first);
        for (const auto &j: i.second) {
            BAASGlobalLogger->BAASInfo("Position            : " + to_string(j.x) + ", " + to_string(j.y));
            BAASGlobalLogger->BAASInfo("Range               : " + to_string(j.r_min) + ", " + to_string(j.r_max) + ", "
                                       + to_string(j.g_min) + ", " + to_string(j.g_max) + ", "
                                       + to_string(j.b_min) + ", " + to_string(j.b_max));
        }
    }
}

void JudgePointRGBRangeFeature::_decode_single_rgb_info(
        const nlohmann::json& j,
        std::vector<RGBInfo> &out
)
{
    if (!j.is_object()) {
        throw JudgePointRGBRangeFeatureError("Single rgb_info should be object position type.");
    }
    if (!j.contains("positions") || !j.contains("range")) {
        throw JudgePointRGBRangeFeatureError("'position' and 'range' should be in rgb_info");
    }
    if (!j["positions"].is_array() || !j["range"].is_array()) {
        throw JudgePointRGBRangeFeatureError("'position' and 'range' should be array");
    }
    if (j["positions"].size() != j["range"].size()) {
        throw JudgePointRGBRangeFeatureError("'position' and 'range' length not match");
    }
    out.clear();
    unsigned int size = j["positions"].size();
    out.resize(size);
    int count = 0;
    for (auto &_p: j["positions"]) {
        if (!_p.is_array() || _p.size() != 2)
            throw JudgePointRGBRangeFeatureError("single position length should be 2");
        out[count].x = _p[0];
        out[count].y = _p[1];
        count++;
    }
    count = 0;
    for (auto &_r: j["range"]) {
        if (!_r.is_array() || _r.size() != 6)
            throw JudgePointRGBRangeFeatureError("single range length should be 6");
        out[count].r_min = _r[0];
        out[count].r_max = _r[1];
        out[count].g_min = _r[2];
        out[count].g_max = _r[3];
        out[count].b_min = _r[4];
        out[count].b_max = _r[5];
        count ++;
    }

}


BAAS_NAMESPACE_END


