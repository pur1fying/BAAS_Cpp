//
// Created by Administrator on 2026/2/18.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BATTLETIMECONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BATTLETIMECONDITION_H_

#include <optional>
#include <string>

#include "BaseCondition.h"
#include "module/auto_fight/workflow/ConditionConfig.h"

BAAS_NAMESPACE_BEGIN

class BattleTimeCondition : public BaseCondition {

public:

    using Op = NumericConditionOp;
    static constexpr auto OVER = Op::OVER;
    static constexpr auto BELOW = Op::BELOW;
    static constexpr auto IN_RANGE = Op::IN_RANGE;
    static constexpr auto INCREASE = Op::INCREASE;
    static constexpr auto DECREASE = Op::DECREASE;

    BattleTimeCondition(
            BAAS* baas,
            auto_fight_d* data,
            const ConditionConfig& config,
            const ConditionIndex& indices = {}
    );

    void reset_state() override;

    void display() const noexcept override;

    void set_d_update_flag() override;


    std::optional<bool> try_match() override;

private:

    Op _op;

    int _value, _range_min, _range_max;

    std::string _time_str;

    std::optional<int> _last_recorded_time;

    int _time_increment;

    bool is_first_frame;
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BATTLETIMECONDITION_H_
