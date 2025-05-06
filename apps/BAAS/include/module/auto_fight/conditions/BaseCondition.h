//
// Created by Administrator on 2025/5/6.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BASECONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BASECONDITION_H_

#include <config/BAASConfig.h>
#include <BAAS.h>

#include "module/auto_fight/screenshot_data/screenshot_data_recoder.h"

BAAS_NAMESPACE_BEGIN

class BaseCondition {

public:
    enum ConditionType {
        COMBINED,
        COST,
        SKILL_NAME,
        SKILL_COST,
        ACC_PHASE,
        AUTO_STATE,
        BOSS_HEALTH
    };

    BaseCondition(BAAS*baas, screenshot_data* data, const BAASConfig& config);

    virtual std::optional<bool> try_match();

    virtual void reset_state();

    virtual void display();

    virtual ~BaseCondition();

    inline bool is_primitive() const noexcept {
        return _is_primitive;
    }

    inline std::optional<bool> is_matched() {
        return _is_matched;
    }

    inline bool has_or_cond() const noexcept {
        return !or_conditions.empty();
    }

    inline bool has_and_cond() const noexcept {
        return !and_conditions.empty();
    }

private:
    static const std::map<std::string, BaseCondition::ConditionType> condition_type_map;

    bool _is_primitive;

    screenshot_data* data;

    BAASConfig config;

    std::vector<uint64_t> or_conditions;
    std::vector<uint64_t> and_conditions;

    std::optional<bool> _is_matched;

    long long cond_j_start_t;

    long long timeout;

    std::string name;

    std::string desc;

};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BASECONDITION_H_
