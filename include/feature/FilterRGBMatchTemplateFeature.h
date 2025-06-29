//
// Created by pc on 2024/8/8.
//

#ifndef BAAS_FEATURE_FILTERRGBMATCHTEMPLATEFEATURE_H_
#define BAAS_FEATURE_FILTERRGBMATCHTEMPLATEFEATURE_H_

#include "BAAS.h"
#include "BAASImageResource.h"
#include "feature/BaseFeature.h"

#define BAAS_FILTER_RGB_MATCH_TEMPLATE_FEATURE 1

BAAS_NAMESPACE_BEGIN

class FilterRGBMatchTemplateFeature : public BaseFeature {

public:

    explicit FilterRGBMatchTemplateFeature(BAASConfig* config);

    bool appear(
            const BAAS* baas,
            BAASConfig& output
    );

    void show() override;

    [[nodiscard]] double self_average_cost(
            const BAAS* baas
    ) override;

    inline void get_template_image(const BAAS* baas, BAASImage& out) {
        std::string resource_ptr = baas->image_resource_prefix + group_name;
        if (resource->is_loaded(resource_ptr)) {
            resource->get(resource_ptr, out);
            return;
        }
        resource->get(resource_ptr, out);
    }

private:

    std::string template_group, template_name, group_name;

    cv::Vec3b mean_rgb_diff;

    double threshold;

    bool check_mean_rgb = false;
};

class  FilterRGBMatchTemplateError : public std::exception {

public:

    explicit FilterRGBMatchTemplateError(const std::string& message) : message(message) {}

    [[nodiscard]] const char *what() const noexcept override
    {
        return message.c_str();
    }

private:

    std::string message;
};
BAAS_NAMESPACE_END

#endif //BAAS_FEATURE_FILTERRGBMATCHTEMPLATEFEATURE_H_
