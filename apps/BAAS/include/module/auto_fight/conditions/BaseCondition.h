//
// Created by Administrator on 2025/5/6.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BASECONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BASECONDITION_H_


#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "BAAS.h"
#include "module/auto_fight/workflow/ConditionConfig.h"
#include "module/auto_fight/workflow/WorkflowNames.h"
#include "module/auto_fight/auto_fight_d.h"

BAAS_NAMESPACE_BEGIN

using ConditionIndex = std::map<std::string, std::uint64_t>;

class BaseCondition {

public:

    static constexpr auto A_COMBINED = ConditionType::AND_COMBINED;
    static constexpr auto O_COMBINED = ConditionType::OR_COMBINED;
    static constexpr auto COST = ConditionType::COST;
    static constexpr auto SKILL_NAME = ConditionType::SKILL_NAME;
    static constexpr auto BOSS_HEALTH = ConditionType::BOSS_HEALTH;
    static constexpr auto BATTLE_TIME = ConditionType::BATTLE_TIME;

    BaseCondition(BAAS* baas, auto_fight_d* data, const ConditionConfig& config,
            const ConditionIndex& indices = {});

    virtual std::optional<bool> try_match();

    virtual void reset_state();

    virtual void set_d_update_flag();

    virtual ~BaseCondition();

    virtual void display() const noexcept;

    void set_and_cond(const std::vector<uint64_t>& and_cond)
    {
        and_conditions = and_cond;
    }

    void set_or_cond(const std::vector<uint64_t>& or_cond)
    {
        or_conditions = or_cond;
    }

    inline const std::vector<uint64_t>& get_and_cond() const noexcept
    {
        return and_conditions;
    }

    inline const std::vector<uint64_t>& get_or_cond() const noexcept
    {
        return or_conditions;
    }

    inline bool has_or_cond() const noexcept
    {
        return !or_conditions.empty();
    }

    inline bool has_and_cond() const noexcept
    {
        return !and_conditions.empty();
    }

    inline long long get_timeout()
    {
        return timeout;
    }


protected:

    void _display_basic_info() const noexcept;

    BAAS* baas;
    BAASLogger* logger;
    auto_fight_d* data;
    std::vector<uint64_t> or_conditions;
    std::vector<uint64_t> and_conditions;

    long long timeout;

    ConditionType type;

    std::string desc;

};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BASECONDITION_H_
