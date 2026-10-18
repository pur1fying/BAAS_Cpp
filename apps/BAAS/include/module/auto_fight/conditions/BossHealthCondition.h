//
// Created by Administrator on 2025/5/15.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BOSSHEALTHCONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BOSSHEALTHCONDITION_H_

#include <optional>

#include "BaseCondition.h"
#include "module/auto_fight/workflow/ConditionConfig.h"

BAAS_NAMESPACE_BEGIN

class BossHealthCondition : public BaseCondition {

public:

    BossHealthCondition(
            BAAS* baas,
            auto_fight_d* data,
            const ConditionConfig& config,
            const ConditionIndex& indices = {}
    );

    using Op = BossHealthConditionOp;
    static constexpr auto C_OVER = Op::CURRENT_OVER;
    static constexpr auto C_BELOW = Op::CURRENT_BELOW;
    static constexpr auto C_IN_RANGE = Op::CURRENT_IN_RANGE;
    static constexpr auto C_INCREASE = Op::CURRENT_INCREASE;
    static constexpr auto C_DECREASE = Op::CURRENT_DECREASE;
    static constexpr auto M_EQUAL = Op::MAX_EQUAL;

    void reset_state() override;

    void display() const noexcept override;

    void set_d_update_flag() override;


    std::optional<bool> try_match() override;

private:

    Op _op;

    double _value, _range_min, _range_max;

    std::optional<double> _last_recorded_health;

    double _health_increment;
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BOSSHEALTHCONDITION_H_

