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

class BAASFeature {
public:

    static void show();

    static void init_feature_ptr();

    static BAASFeature* get_instance();

    static BaseFeature* get_feature_ptr(const std::string& feature_name);

    static bool reset_then_feature_appear(
            BAAS* baas,
            const std::string& feature_name
    );

    static bool feature_appear(
            BAAS* baas,
            const std::string& feature_name,
            BAASConfig& output,
            bool show_log = false
    );

    static std::vector<std::string> get_feature_list();

    inline static bool contains(const std::string& feature_name) {
        return !(features.find(feature_name) == features.end());
    }

private:

    static void load();

    static int load_from_json(const std::filesystem::path &json_path);

    BAASFeature();

    static std::map<std::string, std::unique_ptr<BaseFeature>> features;

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

