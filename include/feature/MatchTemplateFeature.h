//
// Created by pc on 2024/8/6.
//

#ifndef BAAS_FEATURE_MATCHTEMPLATEFEATURE_H_
#define BAAS_FEATURE_MATCHTEMPLATEFEATURE_H_

#include "feature/BAASFeature.h"
#include "BAASImageResource.h"

#define BAAS_MATCH_TEMPLATE_FEATURE 0

class MatchTemplateFeature {
public:
    static bool compare(BAASConfig* parameter, const cv::Mat& image, BAASConfig& output);

    static void get_image(BAASConfig* parameter, BAASImage& image);
};

#endif //BAAS_FEATURE_MATCHTEMPLATEFEATURE_H_
