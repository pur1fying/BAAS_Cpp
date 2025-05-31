//
// Created by Administrator on 2025/5/17.
//


#include "module/auto_fight/actions/auto_fight_act.h"

#include "module/auto_fight/actions/auto_handler.h"
#include "module/auto_fight/actions/acc_handler.h"
#include "module/auto_fight/actions/skill_handler.h"
#include "module/auto_fight/actions/restart_handler.h"
#include "module/auto_fight/actions/skip_ani_handler.h"

BAAS_NAMESPACE_BEGIN

auto_fight_act::auto_fight_act(BAAS* baas, auto_fight_d* data)
{
    this->baas = baas;
    this->data = data;
    this->logger = baas->get_logger();
    auto _it = data->d_fight.find("actions");
    if (_it == data->d_fight.end()) {
        logger->BAASError("Workflow json must contain [ actions ].");
        throw ValueError("Workflow Actions Not Found");
    }
    if (_it->type() != nlohmann::json::value_t::object) {
        logger->BAASError("Workflow [ actions ] config must be a object.");
        throw TypeError("Invalid [ actions ] Config Type.");
    }
    data->d_fight.getBAASConfig("actions", act_config);
}

bool auto_fight_act::_execute(uint64_t act_id) noexcept
{
    assert(act_id < all_act.size());

    for(auto& i : all_act[act_id]) if(!i->execute()) return false;

    return true;
}

void auto_fight_act::_action_pre_check()
{
    uint64_t act_cnt = 0;
    for(const auto& [key, value] : act_config.get_config().items()) {
        if(!value.is_array()) {
            logger->BAASError("Workflow [ single action ] config must be an array.");
            logger->BAASError("Error action key : [ " + key + " ]");
            throw TypeError("Invalid [ single action ] Config Type.");
        }
        act_name_idx_map[key] = act_cnt;
        ++act_cnt;
    }
}

void auto_fight_act::_init_all_act()
{
    BAASConfig sing_act_config;

    for(auto& i : act_config.get_config().items()) {
        sing_act_config = BAASConfig(i.value(), logger);
        _init_single_act(sing_act_config, i.key());
    }
}

void auto_fight_act::_init_single_act(const BAASConfig& config, const std::string& key)
{
    BAASConfig single_act_config;
    all_act.emplace_back();
    for(auto& i : config.get_config()) {
        single_act_config = BAASConfig(i, logger);
        _type_name = single_act_config.getString("t");
        if(!base_handler::is_valid_action_type(_type_name)) {
            logger->BAASError("Invalid Action Type : [ " + _type_name + " ]");
            base_handler::_display_valid_action_types(logger);
            throw ValueError("Invalid Action Type.");
        }
        base_handler::ACTION_TYPE tp = base_handler::act_type_st_to_enum(_type_name);

        switch(tp){
            case base_handler::ACTION_TYPE::ACC:
                all_act.back().push_back(std::make_unique<acc_handler>(baas, data, single_act_config));
                break;
            case base_handler::ACTION_TYPE::AUTO:
                all_act.back().push_back(std::make_unique<auto_handler>(baas, data, single_act_config));
                break;
            case base_handler::ACTION_TYPE::SKILL:
                all_act.back().push_back(std::make_unique<skill_handler>(baas, data, single_act_config));
                break;
            case base_handler::ACTION_TYPE::RESTART:
                all_act.back().push_back(std::make_unique<restart_handler>(baas, data, single_act_config));
                break;
            case base_handler::ACTION_TYPE::SKIP_ANIMATION:
                all_act.back().push_back(std::make_unique<skip_ani_handler>(baas, data, single_act_config));
                break;
        }
    }
}

BAAS_NAMESPACE_END


