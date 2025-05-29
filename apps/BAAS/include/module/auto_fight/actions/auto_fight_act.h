//
// Created by Administrator on 2025/5/17.
//
#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_AUTO_FIGHT_ACT_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_AUTO_FIGHT_ACT_H_

#include "module/auto_fight/actions/base_handler.h"
#include "module/auto_fight/auto_fight_d.h"

BAAS_NAMESPACE_BEGIN

class auto_fight_act {

public:

    auto_fight_act(
            BAAS* baas,
            auto_fight_d* data
    );

    bool _execute(uint64_t act_id) noexcept;

    inline bool act_exist(const std::string& act_name) {
        return act_name_idx_map.find(act_name) != act_name_idx_map.end();
    }

    inline uint64_t get_act_id(const std::string& act_name) {
        auto it = act_name_idx_map.find(act_name);
        if (it != act_name_idx_map.end())return it->second;
        else throw ValueError("Action [ " + act_name + " ] not found.");
    }

private:

    void _init_all_act();

    void _init_single_act(const BAASConfig& config, const std::string& key);

    std::string _type_name;

    BAAS* baas;

    BAASLogger* logger;

    auto_fight_d* data;

    BAASConfig act_config;

    std::map<std::string, uint64_t> act_name_idx_map;

    // each act is a list of handle
    std::vector<std::vector<std::unique_ptr<base_handler>>> all_act;

};


BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_AUTO_FIGHT_ACT_H_