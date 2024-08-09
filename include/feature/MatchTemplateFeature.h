//
// Created by pc on 2024/8/6.
//

#ifndef BAAS_FEATURE_MATCHTEMPLATEFEATURE_H_
#define BAAS_FEATURE_MATCHTEMPLATEFEATURE_H_

#include "feature/BaseFeature.h"
#include "BAASImageResource.h"

#define BAAS_MATCH_TEMPLATE_FEATURE 0

class MatchTemplateFeature : public BaseFeature {
public:
    explicit MatchTemplateFeature(BAASConfig* config);

    static bool compare(BAASConfig* parameter, const cv::Mat& image, BAASConfig& output);

    [[nodiscard]] double self_average_cost(const cv::Mat &image, const std::string& server, const std::string& language) const override;
private:

};

#endif //BAAS_FEATURE_MATCHTEMPLATEFEATURE_H_
