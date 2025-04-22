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
    log.emplace_back("Compare Method :[ JudgePointRGBRangeFeature ].");
    log.push_back("Parameters : " + config->get_config().dump(4));

    cv::Mat image = baas->latest_screenshot;

    int col = max(image.cols, image.rows);
    double ratio = double(col) * 1.0 / 1280;

    log.emplace_back("Screenshot Info : ");
    log.push_back("Image size : " + to_string(image.cols) + "x" + to_string(image.rows));
    log.push_back("Image ratio : " + to_string(ratio));
    int dir = 1;
    if (image.cols > image.rows) dir = 0;
    log.push_back("Image direction : " + to_string(dir));
    int feature_dir = config->getInt("direction", 1);  // 720x1280
    if (dir != feature_dir) {
        log.emplace_back("Screenshot direction not match, Quit.");
        output.insert("log", log);
        return false;
    }

    string server = config->getString("server");
    string language = config->getString("language");
    string server_language = server + "_" + language;
    auto positions = config->get<vector<pair<int, int>>>("/rgb_range/" + server_language + "/positions");
    auto ranges = config->get<vector<vector<uint8_t>>>("/rgb_range/" + server_language + "/range");
    bool check_around = config->getBool("check_around", false);
    int around_range = config->getInt("around_range", 0);

    for (int i = 0; i < positions.size(); i++) {
        positions[i].first = int(positions[i].first * ratio);
        positions[i].second = int(positions[i].second * ratio);
        if (!BAASImageUtil::judge_rgb_range(image, positions[i], ranges[i], check_around, around_range)) {
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
    vector<pair<int, int>> temp_position;
    vector<vector<int>> temp_range;
    int size1, size2;
    vector<int> temp;
    for (auto &i: j["rgb_range"].items()) {
        size1 = size2 = 0;
        for (auto &_p: i.value()["positions"]) {
            if (!_p.is_array() || _p.size() != 2)
                throw JudgePointRGBRangeFeatureError(
                        "Feature format error, position size should be 2"
                );
            temp_position.emplace_back(_p[0],_p[1]);
            size1++;
        }
        for (auto &_r: i.value()["range"]) {
            if (!_r.is_array() || _r.size() != 6)
                throw JudgePointRGBRangeFeatureError("Feature format error range size should be 6");
            temp = {_r[0], _r[1], _r[2], _r[3], _r[4], _r[5]};
            temp_range.emplace_back(temp);
            size2++;
        }
        if (size1 != size2)throw JudgePointRGBRangeFeatureError("Feature format error, size not match");
        position[i.key()] = temp_position;
        rgb_range[i.key()] = temp_range;
    }
}

double JudgePointRGBRangeFeature::self_average_cost(const baas::BAAS *baas)
{
    string server_language = baas->rgb_feature_key;
    auto it = position.find(server_language);
    return 1.0 * double(it->second.size() * 8);
}


BAAS_NAMESPACE_END


