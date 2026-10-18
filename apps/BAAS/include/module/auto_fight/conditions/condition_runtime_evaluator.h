#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_CONDITION_RUNTIME_EVALUATOR_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_CONDITION_RUNTIME_EVALUATOR_H_

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "module/auto_fight/conditions/BaseCondition.h"

BAAS_NAMESPACE_BEGIN


[[nodiscard]] std::optional<bool> evaluate_baas_condition_recursive(
    const std::vector<std::unique_ptr<BaseCondition>>& conditions,
    std::uint64_t condition_index,
    std::vector<std::optional<bool>>& matched,
    std::vector<std::optional<bool>>& self_matched,
    std::vector<bool>& checked);

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_CONDITION_RUNTIME_EVALUATOR_H_
