//
// Created by Administrator on 2025/5/22.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_ANDCOMBINEDCONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_ANDCOMBINEDCONDITION_H_

#include <optional>

#include "BaseCondition.h"
#include "module/auto_fight/workflow/ConditionConfig.h"

BAAS_NAMESPACE_BEGIN

class AndCombinedCondition : public BaseCondition {

public:

    AndCombinedCondition(
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

};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_ANDCOMBINEDCONDITION_H_
