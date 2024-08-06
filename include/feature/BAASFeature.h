//
// Created by pc on 2024/8/6.
//

/*
 *  This class is bound to define any feature you want, use json as in/out to ensure flexibility.
 *  this feature can be image, ocr result, or deep learning model.
 */
#ifndef BAAS_FEATURE_BAASFEATURE_H_
#define BAAS_FEATURE_BAASFEATURE_H_

#include <functional>

#include "config/BAASConfig.h"
#include "feature/MatchTemplateFeature.h"
#include "device/BAASConnection.h"

class BAASFeature {
public:
    static BAASFeature *get_instance();

    static void show();

    static bool appear(BAASConnection* connection,const std::string& name ,const cv::Mat &image, BAASConfig &output, bool show_log = false);

    static bool appear(const std::string& server,const std::string& language,const std::string& name,const cv::Mat &image, BAASConfig &output, bool show_log = false);
private:
    static void init_funcs();

    static void load();

    static int load_from_json(const std::string &path);

    BAASFeature();

    // parameters, image from emulator, output, return means the feature is found or not
    static std::vector<std::function<bool (BAASConfig* parameters, const cv::Mat& image, BAASConfig& output)>> compare_functions;

    static std::map<std::string, BAASConfig*> features;

    static BAASFeature *instance;
};

extern BAASFeature *features;

#endif //BAAS_FEATURE_BAASFEATURE_H_
