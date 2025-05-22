//
// Created by Administrator on 2025/5/17.
//


#include "module/auto_fight/actions/auto_fight_act.h"

#include "module/auto_fight/actions/auto_handler.h"
#include "module/auto_fight/actions/acc_handler.h"
#include "module/auto_fight/actions/skill_handler.h"
#include "module/auto_fight/actions/restart_handler.h"

BAAS_NAMESPACE_BEGIN

auto_fight_act::auto_fight_act(BAAS* baas, auto_fight_d* data)
{
    this->baas = baas;
    this->data = data;
    this->logger = baas->get_logger();
    data->d_fight.getBAASConfig("actions", act_config);
    _init_all_act();
}

void auto_fight_act::execute(uint64_t act_id)
{
    if(act_id >= all_act.size()) {
        logger->BAASWarn("Action ID out of range.");
        throw ValueError("Action ID out of range.");
    }

    for(auto& i : all_act[act_id]) i->execute();
}

void auto_fight_act::_init_all_act()
{
    if(act_config.get_config().type() != nlohmann::json::value_t::object) {
        logger->BAASWarn("Actions config is must be a object.");
        throw TypeError("Invalid Actions Config Type.");
    }

    BAASConfig sing_act_config;

    for(auto& i : act_config.get_config().items()) {
        sing_act_config = BAASConfig(i.value(), logger);
        _init_single_act(sing_act_config);
        act_name_idx_map[i.key()] = all_act.size() - 1;
    }
}

void auto_fight_act::_init_single_act(const BAASConfig& config)
{
    if(!config.get_config().is_array()) {
        logger->BAASWarn("In workflow [ Actions ], Single [ Action ] config must be an array.");
        throw TypeError("Invalid Single [ Action ] Config Type.");
    }

    BAASConfig single_act_config;
    all_act.push_back(std::vector<std::unique_ptr<base_handler>>());
    for(auto& i : config.get_config()) {
        single_act_config = BAASConfig(i, logger);
        _type_name = single_act_config.getString("t");
        if(!base_handler::is_valid_action_type(_type_name)) {
            logger->BAASWarn("Invalid Action Type : [ " + _type_name + " ]");
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
        }
    }
}

BAAS_NAMESPACE_END


