//
// Created by Administrator on 2025/5/16.
//

#include "module/auto_fight/conditions/SkillNameCondition.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "module/auto_fight/workflow/ConditionConfig.h"
#include "module/auto_fight/workflow/WorkflowNames.h"

BAAS_NAMESPACE_BEGIN

SkillNameCondition::SkillNameCondition(
        BAAS* baas,
        auto_fight_d* data,
        const ConditionConfig& config,
        const ConditionIndex& indices
) : BaseCondition(baas, data, config, indices)
{
    type = SKILL_NAME;
    _op = config.skill_op;
    _skill_name = config.skill_name;
    _skill_idx = data->skill_name_to_index_map.at(config.skill_name);
    _p = config.position;
}

void SkillNameCondition::reset_state()
{

}

void SkillNameCondition::display() const noexcept
{
    _display_basic_info();
    logger->BAASInfo("Op      : " + std::string(wire_name(_op)));
    switch (_op) {
        case AT_READY:
        case AT:
            logger->BAASInfo("P       : " + std::to_string(_p));
        case APPEAR_READY:
        case APPEAR:
            logger->BAASInfo("Name    : " + _skill_name);
            break;
    }
}

void SkillNameCondition::set_d_update_flag()
{
    data->d_updater_mask |= (1LL << 2);

    switch (_op) {
        case AT_READY:
            data->stop_when_skill_is_activate[_skill_idx] = true;
        case AT: {
            std::vector<int>& v = data->each_slot_possible_templates[_p];
            if(std::find(v.begin(), v.end(), _skill_idx) == v.end())
                v.push_back(_skill_idx);
            break;
        }
        case APPEAR_READY:
            data->stop_when_skill_is_activate[_skill_idx] = true;
        case APPEAR: {
            for (auto& _v : data->each_slot_possible_templates) {
                if (std::find(_v.begin(), _v.end(), _skill_idx) == _v.end())
                    _v.push_back(_skill_idx);
            }
            break;
        }

    }
}

std::optional<bool> SkillNameCondition::try_match()
{
    switch (_op){
        // Skill name in specified slot
        case AT: {
            if (!data->skills[_p].index.has_value())          return std::nullopt;
            if (data->skills[_p].index.value() != _skill_idx) return false;
            else                                              return true;
        }
        // Skill name appear
        case APPEAR: {
            bool all_dissatisfied = true;
            for (int i = 0; i < data->slot_count; i++) {
                if (!data->skills[i].index.has_value()) {
                    all_dissatisfied = false;
                    continue;
                }
                if(data->skills[i].index.value() == _skill_idx) return true;
            }
            if(all_dissatisfied) return false;
            return std::nullopt;
        }

        // Skill name match and is_active
        case AT_READY: {
            if (!data->skills[_p].index.has_value() || !data->skills[_p].is_active.has_value())
                return std::nullopt;
            if (data->skills[_p].index.value() != _skill_idx) return false;
            // skill inactive, return nullopt to wait for next round check
            if (data->skills[_p].is_active.value() == false)  return std::nullopt;
            else                                              return true;
        }

        case APPEAR_READY: {
            bool all_dissatisfied = true;
            for (int i = 0; i < data->slot_count; i++) {
                if (!data->skills[i].index.has_value() || !data->skills[i].is_active.has_value()) {
                    all_dissatisfied = false;
                    continue;
                }
                if(data->skills[i].index.value() == _skill_idx) {
                    // skill inactive, return nullopt to wait for next round check
                    if (data->skills[i].is_active.value() == false)  return std::nullopt;
                    else                                             return true;
                }
            }
            if(all_dissatisfied) return false;
            return std::nullopt;
        }
    }
}


BAAS_NAMESPACE_END


