//
// Created by Administrator on 2025/5/6.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_COSTCONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_COSTCONDITION_H_

#include "BaseCondition.h"

BAAS_NAMESPACE_BEGIN

class CostCondition : public BaseCondition {
public:
    CostCondition(
            BAAS* baas,
            screenshot_data* data,
            const BAASConfig& config
    );

    enum Op {
        OVER,
        BELOW,
        IN_RANGE,
        INCREASE,
        DECREASE
    };

    void reset_state() override;

    const static std::map<std::string, Op> op_map;

    std::optional<bool> try_match() override;

private:

    void _parse_config_value();

    void _parse_config_range();

    void _parse_op();

    Op _op;

    bool _reset_cost;

    double _value, _range_min, _range_max;

    std::optional<double> _last_recorded_cost;

    double _cost_increment;
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_COSTCONDITION_H_
