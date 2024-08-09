//
// Created by pc on 2024/8/8.
//

#ifndef BAAS_FEATURE_FILTERRGBMATCHTEMPLATEFEATURE_H_
#define BAAS_FEATURE_FILTERRGBMATCHTEMPLATEFEATURE_H_

#include "feature/BaseFeature.h"
#include "BAASImageResource.h"

#define BAAS_FILTER_RGB_MATCH_TEMPLATE_FEATURE 1
class FilterRGBMatchTemplateFeature : public BaseFeature {
public:

    explicit FilterRGBMatchTemplateFeature(BAASConfig* config);

    static bool compare(BAASConfig* parameter, const cv::Mat& image, BAASConfig& output);

    [[nodiscard]] double self_average_cost(const cv::Mat &image, const std::string& server, const std::string& language) const override;

private:

};

#endif //BAAS_FEATURE_FILTERRGBMATCHTEMPLATEFEATURE_H_
