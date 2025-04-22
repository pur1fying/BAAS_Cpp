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
    template_group = config->getString("group");
    assert(!template_group.empty());
    template_name = config->getString("name");
    assert(!template_name.empty());
    if(config->contains("mean_rgb_diff")) {
        if(config->value_type("mean_rgb_diff") != json::value_t::array) {
            throw MatchTemplateError("parma mean_rgb_diff should be array");
        }
        unsigned char size = config->get_array_size("mean_rgb_diff");
        if (size != 3) {
            throw MatchTemplateError("length of mean_rgb_diff should be 3 , get " + to_string(size));
        }
        for (int i = 0; i < 3; i++) {
            if(config->value_type("/mean_rgb_diff/" + to_string(i)) != json::value_t::number_unsigned) {
                throw MatchTemplateError("mean_rgb_diff value should be unsigned int");
            }
            mean_rgb_diff[i] = config->getUInt8("/mean_rgb_diff/" + to_string(i));
        }
    }
    else {
        mean_rgb_diff[0] = 10;
        mean_rgb_diff[1] = 10;
        mean_rgb_diff[2] = 10;
    }

    if(config->contains("threshold")) {
        if(config->value_type("threshold") != json::value_t::number_float) {
            throw MatchTemplateError("parma threshold should be float");
        }
        threshold = config->getDouble("threshold");
    }
    else {
        threshold = 0.8;
    }
    group_name = template_name + "." + template_group;


}

bool MatchTemplateFeature::appear(
        const BAAS *baas,
        BAASConfig &output
)
{
    vector<string> log;
    log.emplace_back("Compare Method : [ MatchTemplateFeature ].");
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

    if (check_mean_rgb) {
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
        if (diff[0] > mean_rgb_diff[0] || diff[1] > mean_rgb_diff[1] || diff[2] > mean_rgb_diff[2]) {
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


    if (maxVal < threshold) {
        log.emplace_back("maxVal < threshold " + to_string(threshold) + " , Quit.");
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

void MatchTemplateFeature::show()
{
    BAASGlobalLogger->sub_title("MatchTemplateFeature");
    BAASGlobalLogger->BAASInfo("is_primitive        : [ " + std::to_string(_is_primitive) + " ]");
    BAASGlobalLogger->BAASInfo("and_feature_count   : [ " + std::to_string(and_feature_ptr.size()) + " ]");
    BAASGlobalLogger->BAASInfo("or_feature_count    : [ " + std::to_string(or_feature_ptr.size()) + " ]");
    BAASGlobalLogger->BAASInfo("Group_name          : [ " + group_name + " ]");
    BAASGlobalLogger->BAASInfo("Check mean_rgb      : [ " + to_string(check_mean_rgb) + " ]");
    BAASGlobalLogger->BAASInfo("RGB_diff            : [ " + to_string(mean_rgb_diff[0]) + " , " +
                               to_string(mean_rgb_diff[1]) + " , " + to_string(mean_rgb_diff[2]) + " ]");
    BAASGlobalLogger->BAASInfo("Threshold           : [ " + to_string(threshold) + " ]");
}


BAAS_NAMESPACE_END

