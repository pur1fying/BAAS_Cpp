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
 *      "mean_rgb_diff": 10,
 *      "threshold": 0.8
 * }
 */
bool MatchTemplateFeature::compare(BAASConfig* parameter, const cv::Mat &image, BAASConfig &output) {
    vector<string> log;
    log.emplace_back("Compare Method [ MatchTemplateFeature ].");
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


    if (parameter->getBool("check_mean_rgb", false)) {
        Vec3b cropped_diff = BAASImageUtil::getRegionMeanRGB(image, template_image.region);
        Vec3b template_diff = BAASImageUtil::getRegionMeanRGB(template_image.image);
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

    Mat cropped = BAASImageUtil::crop(image, template_image.region);
    Mat result;

    matchTemplate(cropped, template_image.image, result, TM_CCOEFF_NORMED);

    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    log.emplace_back("Match Template Result : ");
    log.push_back("Max Similarity : " + to_string(maxVal));

    if (maxVal < parameter->getDouble("threshold", 0.8)) {
        log.emplace_back("Max Similarity less than threshold, Quit.");
        output.insert("log", log);
        return false;
    }

    json j = json::array();
    output.insert("log", log);
    output.insert("max_similarity", maxVal);
    j.push_back(maxLoc.x + cropped.cols);
    j.push_back(maxLoc.y + cropped.rows);
    output.insert("center", j);
    return true;
}

void MatchTemplateFeature::get_image(BAASConfig* parameter, BAASImage &image) {
    string server = parameter->getString("server");
    assert(!server.empty());
    string language = parameter->getString("language");
    assert(!language.empty());
    string group = parameter->getString("group");
    assert(!group.empty());
    string name = parameter->getString("name");
    assert(!name.empty());
    resource->get(server, language, group, name, image);
}