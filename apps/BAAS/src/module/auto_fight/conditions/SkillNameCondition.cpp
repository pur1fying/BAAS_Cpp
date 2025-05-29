//
// Created by Administrator on 2025/5/16.
//

#include "module/auto_fight/conditions/SkillNameCondition.h"

BAAS_NAMESPACE_BEGIN

SkillNameCondition::SkillNameCondition(
        BAAS* baas,
        auto_fight_d* data, 
        const BAASConfig& config
) : BaseCondition(baas, data, config)
{
    type = SKILL_NAME;
    _parse_op();
}

void SkillNameCondition::reset_state()
{

}

void SkillNameCondition::display() const noexcept
{
    _display_basic_info();
    logger->BAASInfo("Op      : [ " + op_st_list[_op] + " ]");
    switch (_op) {
        case AT:
            logger->BAASInfo("P       : [ " + std::to_string(_p) + " ]");
        case APPEAR:
            logger->BAASInfo("Name    : [ " + _skill_name + " ]");
            break;
    }
}

void SkillNameCondition::set_d_update_flag()
{
    data->d_updater_mask |= (1LL << 2);

    switch (_op) {
        case AT: {
            std::vector<int>& v = data->each_slot_possible_templates[_p];
            if(std::find(v.begin(), v.end(), _skill_idx) == v.end())
                v.push_back(_skill_idx);
            break;
        }
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
        case AT: {
            if (!data->skills[_p].index.has_value())          return std::nullopt;
            if (data->skills[_p].index.value() != _skill_idx) return false;
            else                                              return true;
        }
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
    }

    return true;
}

void SkillNameCondition::_parse_op()
{
    std::string op = this->config.getString("op");
    if (op_map.find(op) == op_map.end()) {
        logger->BAASError("Invalid SkillNameCondition op: " + op);
        throw ValueError("Invalid SkillNameCondition op.");
    }
    _op = op_map.at(op);
    switch (_op) {
        case AT:
            _parse_p();
        case APPEAR:
            _parse_skill_name();
    }
}

void SkillNameCondition::_parse_skill_name()
{
    if(!config.contains("name")) {
        logger->BAASError("SkillNameCondition op " + std::to_string(_op) + " requires [ name ].");
        throw ValueError("SkillNameCondition op " + std::to_string(_op) + " requires [ name ].");
    }
    if(config.value_type("name") != nlohmann::detail::value_t::string) {
        logger->BAASError("SkillNameCondition config [ name ] must be string.");
        throw TypeError("SkillNameCondition [ name ] TypeError");
    }
    _skill_name = config.getString("name");

    bool find = false;
    for (int i = 0; i < data->all_possible_skills.size(); i++) {
        if(data->all_possible_skills[i].name == _skill_name) {
            _skill_idx = i;
            find = true;
            break;
        }
    }

    if(!find) {
        logger->BAASError("SkillNameCondition config name [ " + _skill_name +
                          " ] not found in skill template.");
        throw ValueError("Invalid skill name");
    }
}

void SkillNameCondition::_parse_p()
{
    if(!config.contains("p"))   {
        logger->BAASError("SkillNameCondition op " + std::to_string(_op) + " requires [ p ].");
        throw ValueError("SkillNameCondition op " + std::to_string(_op) + " requires [ p ].");
    }
    
    if(config.value_type("p") != nlohmann::json::value_t::number_unsigned) {
        logger->BAASError("SkillNameCondition config [ p ] must be unsigned integer.");
        throw TypeError("SkillNameCondition [ p ] TypeError");
    }
    
    _p = config.getInt("p");
    
    if (_p > (data->slot_count - 1)) {
        logger->BAASError(" value of p [ " + std::to_string(_p) + " ] out of range "
                          " [0, " + std::to_string(data->slot_count - 1) + "]");
        throw ValueError("SkillNameCondition value of [ p ] out of range.");
    }
    
}


BAAS_NAMESPACE_END


