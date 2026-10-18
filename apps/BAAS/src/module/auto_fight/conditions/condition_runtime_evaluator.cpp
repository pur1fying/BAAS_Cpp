#include "module/auto_fight/conditions/condition_runtime_evaluator.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "module/auto_fight/conditions/BaseCondition.h"

BAAS_NAMESPACE_BEGIN
namespace {

class ConditionRuntimeEvaluator {
public:
    ConditionRuntimeEvaluator(
        const std::vector<std::unique_ptr<BaseCondition>>& conditions,
        std::vector<std::optional<bool>>& matched,
        std::vector<std::optional<bool>>& self_matched,
        std::vector<bool>& checked)
        : conditions_(conditions), matched_(matched), self_matched_(self_matched), checked_(checked)
    {
    }

    std::optional<bool> evaluate(std::uint64_t condition_index)
    {
        std::optional<bool> final_result;
        checked_[condition_index] = true;

        std::optional<bool> result;
        if (!self_matched_[condition_index].has_value()) {
            result = conditions_[condition_index]->try_match();
        } else {
            result = self_matched_[condition_index];
        }

        if (!result.has_value()) {
            for (const auto dependency : conditions_[condition_index]->get_and_cond()) {
                if (!pending(dependency)) {
                    if (dissatisfied(dependency)) {
                        final_result = false;
                        break;
                    }
                    continue;
                }
                if (checked_[dependency]) continue;
                result = evaluate(dependency);
                if (result.has_value() && !*result) {
                    final_result = false;
                    break;
                }
            }
            if (final_result.has_value()) {
                matched_[condition_index] = final_result;
                return final_result;
            }
            for (const auto dependency : conditions_[condition_index]->get_or_cond()) {
                if (!pending(dependency)) {
                    if (satisfied(dependency)) {
                        final_result = true;
                        break;
                    }
                    continue;
                }
                if (checked_[dependency]) continue;
                result = evaluate(dependency);
                if (result.has_value() && *result) {
                    final_result = true;
                    break;
                }
            }
        } else {
            // Preserve the BASE assignment exactly: a cached false becomes true
            // after its first observation and affects the next recursive pass.
            self_matched_[condition_index] = true;
            if (*result) {
                bool has_pending = false;
                if (conditions_[condition_index]->has_and_cond()) {
                    for (const auto dependency : conditions_[condition_index]->get_and_cond()) {
                        if (!pending(dependency) || checked_[dependency]) continue;
                        result = evaluate(dependency);
                        if (!result.has_value()) {
                            has_pending = true;
                            break;
                        }
                        if (!*result) {
                            final_result = false;
                            break;
                        }
                    }
                }
                if (final_result != false && !has_pending) final_result = true;
            } else {
                bool has_pending = false;
                if (conditions_[condition_index]->has_or_cond()) {
                    for (const auto dependency : conditions_[condition_index]->get_or_cond()) {
                        if (pending(dependency) || checked_[dependency]) continue;
                        result = evaluate(dependency);
                        if (!result.has_value()) {
                            has_pending = true;
                            break;
                        }
                        if (*result) {
                            final_result = true;
                            break;
                        }
                    }
                }
                if (final_result != true && !has_pending) final_result = false;
            }
        }
        matched_[condition_index] = final_result;
        return final_result;
    }

private:
    bool pending(std::uint64_t index) const
    {
        return !matched_[index].has_value();
    }
    bool satisfied(std::uint64_t index) const
    {
        return matched_[index].value();
    }
    bool dissatisfied(std::uint64_t index) const
    {
        return !matched_[index].value();
    }

    const std::vector<std::unique_ptr<BaseCondition>>& conditions_;
    std::vector<std::optional<bool>>& matched_;
    std::vector<std::optional<bool>>& self_matched_;
    std::vector<bool>& checked_;
};

}  // namespace

std::optional<bool> evaluate_baas_condition_recursive(
    const std::vector<std::unique_ptr<BaseCondition>>& conditions,
    std::uint64_t condition_index,
    std::vector<std::optional<bool>>& matched,
    std::vector<std::optional<bool>>& self_matched,
    std::vector<bool>& checked) {
    return ConditionRuntimeEvaluator{conditions, matched, self_matched, checked}.evaluate(condition_index);
}

BAAS_NAMESPACE_END
