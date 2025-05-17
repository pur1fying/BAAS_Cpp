//
// Created by Administrator on 2025/5/15.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BOSSHEALTHCONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BOSSHEALTHCONDITION_H_

#include "BaseCondition.h"

BAAS_NAMESPACE_BEGIN

class BossHealthCondition : public BaseCondition {

public:

    BossHealthCondition(
            BAAS* baas,
            auto_fight_d* data,
            const BAASConfig& config
    );

    enum Op {
        C_OVER,
        C_BELOW,
        C_IN_RANGE,
        C_INCREASE,
        C_DECREASE,
        M_EQUAL
    };

    void reset_state() override;

    void display() const noexcept override;

    void set_d_update_flag() override;

    const static std::map<std::string, Op> op_map;

    const static std::vector<std::string> op_to_st;

    std::optional<bool> try_match() override;

private:

    void _parse_config_value();

    void _parse_config_range();

    void _parse_op();

    Op _op;

    double _value, _range_min, _range_max;

    std::optional<double> _last_recorded_health;

    double _health_increment;
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BOSSHEALTHCONDITION_H_

