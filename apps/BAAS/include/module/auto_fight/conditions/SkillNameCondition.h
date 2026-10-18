//
// Created by Administrator on 2025/5/16.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_SKILLNAMECONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_SKILLNAMECONDITION_H_

#include <cstdint>
#include <optional>
#include <string>

#include "BaseCondition.h"
#include "module/auto_fight/workflow/ConditionConfig.h"

BAAS_NAMESPACE_BEGIN

class SkillNameCondition : public BaseCondition {

public:

    SkillNameCondition(
            BAAS* baas,
            auto_fight_d* data,
            const ConditionConfig& config,
            const ConditionIndex& indices = {}
    );

    using Op = SkillNameConditionOp;
    static constexpr auto APPEAR = Op::APPEAR;
    static constexpr auto AT = Op::AT;
    static constexpr auto APPEAR_READY = Op::APPEAR_READY;
    static constexpr auto AT_READY = Op::AT_READY;

    void reset_state() override;

    void display() const noexcept override;

    void set_d_update_flag() override;


    std::optional<bool> try_match() override;

private:

    Op _op;

    std::string _skill_name;

    uint64_t _skill_idx;

    uint64_t _p;

};


BAAS_NAMESPACE_END


#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_SKILLNAMECONDITION_H_
