//
// Created by Administrator on 2025/5/16.
//

#include "module/auto_fight/conditions/SkillNameCondition.h"

#include "module/auto_fight/constants.h"

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
    auto it = config.find("op");
    if (it == config.end()) {
        logger->BAASError("[ SkillNameCondition ] config must contain [ op ].");
        _log_valid_op("[ SkillNameCondition ] [ op ]", logger, op_st_list);
        throw ValueError("[ SkillNameCondition ] [ op ] not found.");
    }
    if (!it->is_string()) {
        logger->BAASError("[ SkillNameCondition ] [ op ] must be a string.");
        _log_valid_op("[ SkillNameCondition ] [ op ]", logger, op_st_list);
        throw TypeError("[ SkillNameCondition ] [ op ] TypeError");
    }
    std::string op_str = *it;
    auto _it = op_map.find(op_str);
    if (_it == op_map.end()) {
        logger->BAASError("Invalid [ SkillNameCondition ] [ op ] : " + op_str);
        _log_valid_op("[ SkillNameCondition ] [ op ]", logger, op_st_list);
        throw ValueError("Invalid [ SkillNameCondition ] [ op ].");
    }
    _op = _it->second;
    switch (_op) {
        case AT:
            _parse_p();
        case APPEAR:
            _parse_skill_name();
    }
}

void SkillNameCondition::_parse_skill_name()
{
    auto it = config.find("name");
    if ( it == config.end()) {
        logger->BAASError("[ SkillNameCondition ] config must contain [ name ].");
        _log_valid_skill_names("[ SkillNameCondition ] [ name ]", logger, data);
        throw ValueError("[ SkillNameCondition ] [ name ] not found.");
    }
    if (!it->is_string()) {
        logger->BAASError("[ SkillNameCondition ] [ name ] must be a string.");
        _log_valid_skill_names("[ SkillNameCondition ] [ name ]", logger, data);
        throw TypeError("[ SkillNameCondition ] [ name ] TypeError");
    }

    _skill_name = *it;
    bool find = false;
    for (int i = 0; i < data->all_possible_skills.size(); i++) {
        if(data->all_possible_skills[i].name == _skill_name) {
            _skill_idx = i;
            find = true;
            break;
        }
    }

    if(!find) {
        logger->BAASError("[ SkillNameCondition ] [ name ] : \"" + _skill_name + "\" not found in all possible skills.");
        _log_valid_skill_names("[ SkillNameCondition ] [ name ]", logger, data);
        throw ValueError("Invalid [ SkillNameCondition ] [ name ].");
    }
}

void SkillNameCondition::_parse_p()
{
    auto  _it = this->config.find("p");
    if (_it == this->config.end()) {
        logger->BAASError("[ SkillNameCondition ] [ op ] : \"" + op_st_list[_op] + "\" requires [ p ].");
        throw ValueError("[ SkillNameCondition ] [ p ] not found.");
    }
    if (!_it->is_number_unsigned()) {
        logger->BAASError("[ SkillNameCondition ] [ p ] must be an unsigned integer.");
        throw TypeError("[ SkillNameCondition ] [ p ] TypeError");
    }
    
    _p = _it->get<uint64_t>();
    
    if (_p > (data->slot_count - 1)) {
        logger->BAASError("[ SkillNameCondition ] [ p ] : " + std::to_string(_p) + " out of range "
                          " [0, " + std::to_string(data->slot_count - 1) + "]");
        throw ValueError("[ SkillNameCondition ] [ p ] out of range");
    }
}


BAAS_NAMESPACE_END


