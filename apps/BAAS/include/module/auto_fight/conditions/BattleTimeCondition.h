//
// Created by Administrator on 2026/2/18.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BATTLETIMECONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BATTLETIMECONDITION_H_

#include "BaseCondition.h"

BAAS_NAMESPACE_BEGIN

class BattleTimeCondition : public BaseCondition {

public:

    enum Op {
        OVER,
        BELOW,
        IN_RANGE,
        INCREASE,
        DECREASE
    };

    BattleTimeCondition(
            BAAS* baas,
            auto_fight_d* data,
            const BAASConfig& config
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

    int _value, _range_min, _range_max;

    std::string _time_str;

    std::optional<int> _last_recorded_time;

    int _time_increment;
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BATTLETIMECONDITION_H_
