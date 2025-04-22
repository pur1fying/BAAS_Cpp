//
// Created by pc on 2024/8/13.
//

#ifndef BAAS_FEATURE_JUDGEPOINTRGBRANGEFEATURE_H_
#define BAAS_FEATURE_JUDGEPOINTRGBRANGEFEATURE_H_

#include "feature/BaseFeature.h"

#define BAAS_JUDGE_POINT_RGB_RANGE_FEATURE 2

BAAS_NAMESPACE_BEGIN

class JudgePointRGBRangeFeature : public BaseFeature {
private:
    struct RGBInfo{
        int x;
        int y;
        u_char r_min;
        u_char r_max;
        u_char g_min;
        u_char g_max;
        u_char b_min;
        u_char b_max;
    };
public:
    explicit JudgePointRGBRangeFeature(BAASConfig *config);

    static void decode_single_rgb_info(const nlohmann::json& j, std::vector<RGBInfo>& out);

    void show() override;

    bool appear(
            const BAAS *baas,
            BAASConfig &output
    ) override;

    [[nodiscard]] double self_average_cost(
            const BAAS* baas
    ) override;
private:

    std::map<std::string, std::vector<RGBInfo>> rgb_info;
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
