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

#include "feature/BaseFeature.h"

BAAS_NAMESPACE_BEGIN

class BAAS;

class BAASFeature {
public:
    static BAASFeature *get_instance();

    static void show();

    static bool appear(
            BAASConnection *connection,
            const std::string &name,
            const cv::Mat &image,
            BAASConfig &output,
            bool show_log = false
    );

    static bool appear(
            const std::string &server,
            const std::string &language,
            const std::string &name,
            const cv::Mat &image,
            BAASConfig &output,
            bool show_log = false
    );

    static void reset_feature( const std::string &name);

    static BaseFeature *get_feature(const std::string &name);

    static bool reset_then_feature_appear(BAAS* baas, const std::string &feature_name);

    static bool feature_appear(
            BAAS* baas,
            const std::string &feature_name,
            BAASConfig &output,
            bool show_log = false
    );

    static bool feature_appear(
            BAAS* baas,
            const std::string &feature_name
    );

private:

    static void init_funcs();

    static void load();

    static int load_from_json(const std::string &path);

    BAASFeature();

    // parameters, image from emulator, output, return means the feature is found or not
    static std::vector<std::function<bool(
            BAASConfig *parameters,
            const cv::Mat &image,
            BAASConfig &output
    )>> compare_functions;

    static std::map<std::string, BaseFeature *> features;

    static BAASFeature *instance;
};

extern BAASFeature *baas_features;

class BAASFeatureError : public std::exception {
public:
    explicit BAASFeatureError(const std::string &msg) : msg(msg) {}

private:
    std::string msg;
};

BAAS_NAMESPACE_END

#endif //BAAS_FEATURE_BAASFEATURE_H_

