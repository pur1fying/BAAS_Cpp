//
// Created by pc on 2024/8/13.
//

#ifndef BAAS_FEATURE_JUDGEPOINTRGBRANGEFEATURE_H_
#define BAAS_FEATURE_JUDGEPOINTRGBRANGEFEATURE_H_

#include "feature/BaseFeature.h"

#define BAAS_JUDGE_POINT_RGB_RANGE_FEATURE 2

BAAS_NAMESPACE_BEGIN

class JudgePointRGBRangeFeature : public BaseFeature {
public:
    explicit JudgePointRGBRangeFeature(BAASConfig *config);

    bool appear(
            const BAAS *baas,
            BAASConfig &output
    ) override;

    [[nodiscard]] double self_average_cost(
            const BAAS* baas
    ) override;
private:
    std::map<std::string, std::vector<std::pair<int, int>>> position;
    std::map<std::string, std::vector<std::vector<int>>> rgb_range;

};

class JudgePointRGBRangeFeatureError : public std::exception {
public:
    explicit JudgePointRGBRangeFeatureError(const std::string &message) : message(message) {}

    [[nodiscard]] const char *what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};

BAAS_NAMESPACE_END
#endif //BAAS_FEATURE_JUDGEPOINTRGBRANGEFEATURE_H_
