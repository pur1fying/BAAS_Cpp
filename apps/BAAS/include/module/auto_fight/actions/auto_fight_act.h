//
// Created by Administrator on 2025/5/17.
//
#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_AUTO_FIGHT_ACT_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_AUTO_FIGHT_ACT_H_

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "module/auto_fight/workflow/ActionConfig.h"
#include "module/auto_fight/auto_fight_d.h"
#include "module/auto_fight/actions/base_handler.h"

BAAS_NAMESPACE_BEGIN

class auto_fight_act {

public:

    auto_fight_act(
            BAAS* baas,
            auto_fight_d* data
    );

    bool _execute(uint64_t act_id);

    inline auto act_find(const std::string& act_name)
    {
        return act_name_idx_map.find(act_name);
    }

    inline auto act_end() const noexcept
    {
        return act_name_idx_map.end();
    }

    inline uint64_t size()
    {
        return act_name_idx_map.size();
     }

    const WorkflowNameIndex& name_indices() const
    {
        return act_name_idx_map;
    }

    void _init_all_act(const std::map<std::string, std::optional<ActionConfig>>& actions);

private:
    BAAS* baas;

    BAASLogger* logger;

    auto_fight_d* data;


    std::map<std::string, uint64_t> act_name_idx_map;

    // each act is a list of handle
    std::vector<std::vector<std::unique_ptr<base_handler>>> all_act;

};


BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_AUTO_FIGHT_ACT_H_
