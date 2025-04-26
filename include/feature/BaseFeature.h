//
// Created by pc on 2024/8/7.
//

#ifndef BAAS_FEATURE_BASEFEATURE_H_
#define BAAS_FEATURE_BASEFEATURE_H_

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <optional>

#include "config/BAASConfig.h"
#include "BAASImageResource.h"
#include "BAAS.h"

// used to create different feature and combine them

BAAS_NAMESPACE_BEGIN

class BaseFeature {
public:
    inline std::vector<std::string> get_and_features() {
        return this->config->get<std::vector<std::string>>("and_features", {});
    }

    inline std::vector<std::string> get_or_features() {
        return this->config->get<std::vector<std::string>>("or_features", {});
    }

    explicit BaseFeature() = default;

    explicit BaseFeature(BAASConfig *config);

    [[nodiscard]] inline bool is_primitive() const
    {
        return _is_primitive;
    }

    std::vector<BaseFeature *> get_and_feature_ptr();

    std::vector<BaseFeature *> get_or_feature_ptr();

    bool has_and_feature();

    bool has_or_feature();

    virtual void show();

    // compare func
    virtual bool appear(
            const BAAS *baas,
            BAASConfig &output
    );

    // used to judge the compare order of feature

    virtual double self_average_cost(
            const BAAS* baas
    );

    [[nodiscard]] double all_average_cost(
            const BAAS* baas
    );

    [[nodiscard]] inline BAASConfig *get_config()
    {
        return config;
    }

    inline void set_path(const std::string &_path)
    {
        this->path = _path;
    }

    inline const std::string &get_path()
    {
        return path;
    }
protected:
    bool _is_primitive;

    std::vector<BaseFeature*> and_feature_ptr;

    std::vector<BaseFeature*> or_feature_ptr;

    BAASConfig *config;

    std::optional<bool> this_round_result;

    std::string path;

    friend class BAASFeature;
};

BAAS_NAMESPACE_END

#endif //BAAS_FEATURE_BASEFEATURE_H_
