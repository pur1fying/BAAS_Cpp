//
// Created by pc on 2024/8/8.
//

#include "feature/FilterRGBMatchTemplateFeature.h"

using namespace std;
using namespace cv;
using namespace nlohmann;
/* example parameter
 * "server" and "language" should be insert dynamically according to the connection
 * "common_close-notice-button_appear":{
 *   "feature_type": 0,
 *
 *   "group": "common",
 *   "name": "close-notice-button",
 *
 *   "check_mean_rgb": true,
 *   "mean_rgb_diff": [5, 5, 5],
 *   "threshold": 0.9
 * }
 */
FilterRGBMatchTemplateFeature::FilterRGBMatchTemplateFeature(BAASConfig *config) : BaseFeature(config) {

}

bool FilterRGBMatchTemplateFeature::compare(BAASConfig *parameter, const cv::Mat &image, BAASConfig &output) {
    vector<string> log;
    log.emplace_back("Compare Method :[ FilterRGBMatchTemplateFeature ].");
    log.push_back("Parameters : " + parameter->get_config().dump(4));

    BAASImage template_image;
    get_image(parameter, template_image);
    log.emplace_back("Template Image Info : ");
    log.push_back(template_image.gen_info());

    int dir = 1;
    if(image.cols > image.rows) dir= 0;
    log.emplace_back("Screenshot Info : ");
    log.push_back("Image size : " + to_string(image.cols) + "x" + to_string(image.rows));
    log.push_back("Image direction : " + to_string(dir));

    if (template_image.direction != dir) {
        log.emplace_back("Template image and screenshot direction not match, Quit.");
        output.insert("log", log);
        return false;
    }

    Mat cropped = BAASImageUtil::crop(image, template_image.region), mask, filtered;
    BAASImageUtil::gen_not_black_region_mask(template_image.image, mask);
    bitwise_and(cropped, cropped, filtered, mask);

    if (parameter->getBool("check_mean_rgb", true)) {
        Vec3b cropped_diff = BAASImageUtil::get_region_not_black_mean_rgb(filtered);
        Vec3b template_diff = BAASImageUtil::get_region_not_black_mean_rgb(template_image.image);
        log.push_back("Screenshot Mean RGB  : [\t" + to_string(cropped_diff[0]) + " ,\t" + to_string(cropped_diff[1]) + " ,\t" + to_string(cropped_diff[2]) + " ]");
        log.push_back("Template   Mean RGB  : [\t" + to_string(template_diff[0]) + " ,\t" + to_string(template_diff[1]) + " ,\t" + to_string(template_diff[2]) + " ]");
        Vec3b diff = BAASImageUtil::calc_abs_diff(cropped_diff, template_diff);
        auto diff_para = parameter->template get<vector<int>>("mean_rgb_diff", {10, 10, 10});
        if (diff[0] > diff_para[0] || diff[1] > diff_para[1] || diff[2] > diff_para[2]) {
            log.emplace_back("RGB diff too large, Quit.");
            output.insert("log", log);
            return false;
        }
    }

    Mat result;

    matchTemplate(filtered, template_image.image, result, TM_CCOEFF_NORMED);

    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    log.emplace_back("Match Template Result : ");
    log.push_back("Max Similarity : " + to_string(maxVal));

    double threshold_min, threshold_max;
    try{
        threshold_min = parameter->get <double>("/threshold/0");
        threshold_max = parameter->get <double>("/threshold/1");
    } catch (KeyError &e) {
        threshold_min = parameter->getDouble("threshold", 0.8);
        threshold_max = 1.0;
    }
    if (maxVal < threshold_min || maxVal > threshold_max)  {
        log.emplace_back("maxVal not in range [" + to_string(threshold_min) + ", " + to_string(threshold_max) + "], Quit.");
        output.insert("log", log);
        return false;
    }

    json j = json::array();
    output.insert("log", log);
    output.insert("max_similarity", maxVal);
    j.push_back(template_image.region.ul.x + cropped.cols/2);
    j.push_back(template_image.region.ul.x + cropped.rows/2);
    output.insert("center", j);
    return true;
}

double FilterRGBMatchTemplateFeature::self_average_cost(const cv::Mat &image, const std::string &server,const std::string &language) const {
    string group = config->getString("group");
    assert(!group.empty());
    string name = config->getString("name");
    assert(!name.empty());

    BAASImage template_image;
    resource->get(server, language, group, name, template_image);
    return template_image.image.rows * template_image.image.cols;
}
