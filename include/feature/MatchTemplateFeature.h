//
// Created by pc on 2024/8/6.
//

#ifndef BAAS_FEATURE_MATCHTEMPLATEFEATURE_H_
#define BAAS_FEATURE_MATCHTEMPLATEFEATURE_H_

#include "feature/BaseFeature.h"
#include "BAASImageResource.h"

#define BAAS_MATCH_TEMPLATE_FEATURE 0

BAAS_NAMESPACE_BEGIN

class MatchTemplateFeature : public BaseFeature {
public:
    explicit MatchTemplateFeature(BAASConfig *config);

    void show();

    bool appear(
            const BAAS *baas,
            BAASConfig &output
    ) override;

    [[nodiscard]] double self_average_cost(
            const BAAS * baas
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


class  MatchTemplateError : public std::exception {
public:
    explicit MatchTemplateError(const std::string &message) : message(message) {}

    [[nodiscard]] const char *what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};

BAAS_NAMESPACE_END

#endif //BAAS_FEATURE_MATCHTEMPLATEFEATURE_H_
