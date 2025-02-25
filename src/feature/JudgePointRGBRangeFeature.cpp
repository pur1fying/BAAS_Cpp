//
// Created by pc on 2024/8/13.
//
#ifdef BAAS_APP_BUILD_FEATURE

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

JudgePointRGBRangeFeature::JudgePointRGBRangeFeature(BAASConfig *config) : BaseFeature(config)
{
    json j = config->get_config();
    if (!j.is_object() || !j.contains("rgb_range") || !j["rgb_range"].is_object()) {
        throw JudgePointRGBRangeFeatureError("rgb_range Feature format error");
    }
    vector<pair<int, int>> temp_position;
    vector<vector<int>> temp_range;
    json p, r;
    int size1, size2;
    vector<int> temp;
    for (auto &i: j["rgb_range"].items()) {
        size1 = size2 = 0;
        for (auto &j: i.value()["positions"]) {
            if (!j.is_array() || j.size() != 2)
                throw JudgePointRGBRangeFeatureError(
                        "Feature format error, position size should be 2"
                );
            temp_position.emplace_back(make_pair(j[0], j[1]));
            size1++;
        }
        for (auto &j: i.value()["range"]) {
            if (!j.is_array() || j.size() != 6)
                throw JudgePointRGBRangeFeatureError(
                        "Feature format error range size should be 6"
                );
            temp = {j[0], j[1], j[2], j[3], j[4], j[5]};
            temp_range.emplace_back(temp);
            size2++;
        }
        if (size1 != size2)throw JudgePointRGBRangeFeatureError("Feature format error, size not match");
        position[i.key()] = temp_position;
        rgb_range[i.key()] = temp_range;
        self_average_cost_map[i.key()] = temp_position.size() * 8;
    }
}

bool JudgePointRGBRangeFeature::compare(
        BAASConfig *parameter,
        const cv::Mat &image,
        BAASConfig &output
)
{
    vector<string> log;
    log.emplace_back("Compare Method :[ JudgePointRGBRangeFeature ].");
    log.push_back("Parameters : " + parameter->get_config().dump(4));

    int col = max(image.cols, image.rows);
    double ratio = double(col) * 1.0 / 1280;

    log.emplace_back("Screenshot Info : ");
    log.push_back("Image size : " + to_string(image.cols) + "x" + to_string(image.rows));
    log.push_back("Image ratio : " + to_string(ratio));
    int dir = 1;
    if (image.cols > image.rows) dir = 0;
    log.push_back("Image direction : " + to_string(dir));
    int feature_dir = parameter->getInt("direction", 1);  // 720x1280
    if (dir != feature_dir) {
        log.emplace_back("Screenshot direction not match, Quit.");
        output.insert("log", log);
        return false;
    }

    string server = parameter->getString("server");
    string language = parameter->getString("language");
    string server_language = server + "_" + language;
    auto positions = parameter->get<vector<pair<int, int>>>("/rgb_range/" + server_language + "/positions");
    auto ranges = parameter->get<vector<vector<uint8_t>>>("/rgb_range/" + server_language + "/range");
    bool check_around = parameter->getBool("check_around", false);
    int around_range = parameter->getInt("around_range", 0);

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

double JudgePointRGBRangeFeature::self_average_cost(
        const cv::Mat &image,
        const std::string &server,
        const std::string &language
)
{
    string server_language = server + "_" + language;
    auto it = self_average_cost_map.find(server_language);
    if (it == self_average_cost_map.end()) {
        return 0;
    }
    return it->second.value();
}

BAAS_NAMESPACE_END

#endif //BAAS_APP_BUILD_FEATURE