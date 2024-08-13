//
// Created by pc on 2024/8/7.
//

#ifndef BAAS_FEATURE_BASEFEATURE_H_
#define BAAS_FEATURE_BASEFEATURE_H_

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

#include "config/BAASConfig.h"
#include "BAASImageResource.h"

// used to create different feature and combine them

class BaseFeature {
public:
    explicit BaseFeature() = default;

    explicit BaseFeature(BAASConfig* config);

    inline bool is_primitive() {
        return and_features.empty() && or_features.empty();
    }

    // used to judge the compare order of feature
    virtual double self_average_cost(const cv::Mat &image, const std::string& server, const std::string& language);

    std::vector<std::string> get_and_features();

    std::vector<std::string> get_or_features();

    bool has_and_feature();

    bool has_or_feature();

    [[nodiscard]] double all_average_cost(const cv::Mat &image, const std::string& server, const std::string& language);

    [[nodiscard]] inline BAASConfig* get_config() {
        return config;
    }

    static void get_image(BAASConfig* parameter, BAASImage& image);

    inline bool get_this_round_result() {
        assert(this_round_result.has_value());
        return this_round_result.value();
    }

    inline bool is_checked_this_round() {
        return this_round_result.has_value();
    }

    inline bool set_checked_this_round(bool result) {
        this_round_result = result;
        return result;
    }

    inline void reset_checked() {
        this_round_result.reset();
    }

    inline void set_path(const std::string& path) {
        this->path = path;
    }

    inline const std::string& get_path() {
        return path;
    }

    inline void set_enabled(const bool enabled) {
        is_enabled = enabled;
    }

    inline bool get_enabled() {
        return is_enabled;
    }
protected:
    std::vector<std::string> or_features;

    std::vector<std::string> and_features;

    BAASConfig* config;

    std::optional<bool> this_round_result;

    std::string path;

    bool is_enabled;
};



#endif //BAAS_FEATURE_BASEFEATURE_H_
