#ifndef BAAS_CPP_TESTS_WORKFLOW_STUBS_BASECONDITION_H_
#define BAAS_CPP_TESTS_WORKFLOW_STUBS_BASECONDITION_H_

#include <cstdint>
#include <optional>
#include <vector>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

// Test double for the device-independent production recursive evaluator.
class BaseCondition {
public:
    virtual ~BaseCondition() = default;
    virtual std::optional<bool> try_match() = 0;
    const std::vector<std::uint64_t>& get_and_cond() const
    {
        return and_conditions;
    }
    const std::vector<std::uint64_t>& get_or_cond() const
    {
        return or_conditions;
    }
    bool has_and_cond() const
    {
        return !and_conditions.empty();
    }
    bool has_or_cond() const
    {
        return !or_conditions.empty();
    }

    std::vector<std::uint64_t> and_conditions;
    std::vector<std::uint64_t> or_conditions;
};

BAAS_NAMESPACE_END

#endif // BAAS_CPP_TESTS_WORKFLOW_STUBS_BASECONDITION_H_
