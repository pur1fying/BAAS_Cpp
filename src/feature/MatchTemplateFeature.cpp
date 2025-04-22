//
// Created by pc on 2024/8/6.
//


#include "feature/MatchTemplateFeature.h"

using namespace std;
using namespace cv;
using namespace nlohmann;

/* example parameter
 * "server" and "language" should be insert dynamically according to the connection
 * "main_page_draw_card_checked_appear":{
 *      "feature_type": 0,
 *
 *      "server": "JP",
 *      "language": "ja-jp",
 *      "group": "main_page",
 *      "name": "draw-card-checked",
 *
 *      "check_mean_rgb": true,
 *      "mean_rgb_diff": [10, 10, 10],
 *      "threshold": 0.8
 * }
 */

BAAS_NAMESPACE_BEGIN

MatchTemplateFeature::MatchTemplateFeature(BAASConfig *config) : BaseFeature(config)
{
    self_average_cost_map.clear();
    template_group = config->getString("group");
    assert(!template_group.empty());
    template_name = config->getString("name");
    assert(!template_name.empty());
    group_name = template_name + "." + template_group;
}

bool MatchTemplateFeature::appear(
        const BAAS *baas,
        BAASConfig &output
)
{
    vector<string> log;
    log.emplace_back("Compare Method :[ MatchTemplateFeature ].");
    log.push_back("Parameters : " + config->get_config().dump(4));
    BAASImage template_image;

    get_template_image(baas, template_image);
    if (template_image.image.empty()) {
        log.emplace_back("Template Image is empty, Quit.");
        output.insert("log", log);
        return false;
    }
    log.emplace_back("Template Image Info : ");
    log.push_back(template_image.gen_info());

    cv::Mat image = baas->latest_screenshot;

    int dir = 1;
    if (image.cols > image.rows) dir = 0;
    log.emplace_back("Screenshot Info : ");
    log.push_back("Image size : " + to_string(image.cols) + "x" + to_string(image.rows));
    log.push_back("Image direction : " + to_string(dir));

    if (template_image.direction != dir) {
        log.emplace_back("Template image and screenshot direction not match, Quit.");
        output.insert("log", log);
        return false;
    }

    Mat temp = image;

    int longer_edge = max(template_image.image.cols, template_image.image.rows);
    if (longer_edge != 1280) {
        temp = image.clone();
        if (dir == 0) BAASImageUtil::resize(temp, temp, 1280, 720);
        else BAASImageUtil::resize(temp, temp, 720, 1280);
    }

    if (config->getBool("check_mean_rgb", true)) {
        Vec3b cropped_diff = BAASImageUtil::getRegionMeanRGB(temp, template_image.region);
        Vec3b template_diff = BAASImageUtil::getRegionMeanRGB(template_image.image);
        log.push_back(
                "Screenshot Mean RGB  : [\t" + to_string(cropped_diff[0]) + " ,\t" + to_string(cropped_diff[1]) +
                " ,\t" + to_string(cropped_diff[2]) + " ]"
        );
        log.push_back(
                "Template   Mean RGB  : [\t" + to_string(template_diff[0]) + " ,\t" + to_string(template_diff[1]) +
                " ,\t" + to_string(template_diff[2]) + " ]"
        );
        Vec3b diff = BAASImageUtil::calc_abs_diff(cropped_diff, template_diff);
        auto diff_para = config->template get<vector<int>>("mean_rgb_diff", {10, 10, 10});
        if (diff[0] > diff_para[0] || diff[1] > diff_para[1] || diff[2] > diff_para[2]) {
            log.emplace_back("RGB diff too large, Quit.");
            output.insert("log", log);
            return false;
        }
    }

    Mat cropped = BAASImageUtil::crop(temp, template_image.region);
    Mat result;

    matchTemplate(cropped, template_image.image, result, TM_CCOEFF_NORMED);

    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    log.emplace_back("Match Template Result : ");
    log.push_back("Max Similarity : " + to_string(maxVal));

    double threshold_min, threshold_max;
    try {
        threshold_min = config->get<double>("/threshold/0");
        threshold_max = config->get<double>("/threshold/1");
    } catch (KeyError &e) {
        threshold_min = config->getDouble("threshold", 0.8);
        threshold_max = 1.0;
    }
    if (maxVal < threshold_min || maxVal > threshold_max) {
        log.emplace_back(
                "maxVal not in range [" + to_string(threshold_min) + ", " + to_string(threshold_max) + "], Quit."
        );
        output.insert("log", log);
        return false;
    }

    json j = json::array();
    output.insert("log", log);
    output.insert("max_similarity", maxVal);
    j.push_back(template_image.region.ul.x + cropped.cols / 2);
    j.push_back(template_image.region.ul.y + cropped.rows / 2);
    output.insert("center", j);
    return true;
}


double MatchTemplateFeature::self_average_cost(const BAAS *baas)
{
    string image_key = baas->get_image_resource_prefix() + group_name;
    if(!resource->is_loaded(image_key)) return 0;

    BAASImage template_image;
    resource->get(image_key, template_image);
    double average_cost = 1.0 * template_image.image.rows * template_image.image.cols;
    return average_cost;
}


BAAS_NAMESPACE_END

