//
// Created by Administrator on 2025/5/6.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_COSTCONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_COSTCONDITION_H_

#include "BaseCondition.h"

BAAS_NAMESPACE_BEGIN

class CostCondition : public BaseCondition {
public:
    enum Op {
        OVER,
        BELOW,
        IN_RANGE,
        INCREASE,
        DECREASE
    };

    CostCondition(
            BAAS* baas,
            auto_fight_d* data,
            const BAASConfig& config
    );

    CostCondition(
            BAAS* baas,
            auto_fight_d* data,
            Op op,
            double value,
            uint64_t timeout = BAAS_AUTO_FIGHT_CONDITION_DEFAULT_TIMEOUT
    );



    void reset_state() override;

    void display() const noexcept override;

    void set_d_update_flag() override;

    const static std::map<std::string, Op> op_map;

    const static std::vector<std::string> op_st_list;

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
