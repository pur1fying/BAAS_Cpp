//
// Created by Administrator on 2025/5/17.
//


#include "module/auto_fight/actions/auto_fight_act.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "module/auto_fight/actions/ActionFactory.h"

BAAS_NAMESPACE_BEGIN

auto_fight_act::auto_fight_act(BAAS* baas, auto_fight_d* data)
{
    this->baas = baas;
    this->data = data;
    this->logger = baas->get_logger();
}

bool auto_fight_act::_execute(uint64_t act_id)
{
    assert(act_id < all_act.size());

    for(auto& i : all_act[act_id]) if(!i->execute()) return false;

    return true;
}

void auto_fight_act::_init_all_act(
        const std::map<std::string, std::optional<ActionConfig>>& actions
)
{
    act_name_idx_map.clear();
    all_act.clear();
    all_act.reserve(actions.size());
    for (const auto& [name, action] : actions) {
        act_name_idx_map.emplace(name, all_act.size());
        auto& handlers = all_act.emplace_back();
        handlers.reserve(action->steps.size());
        for (const auto& step : action->steps) {
            handlers.push_back(create_baas_action_handler(baas, data, *step));
        }
    }
}

BAAS_NAMESPACE_END


