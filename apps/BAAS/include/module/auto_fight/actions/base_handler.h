//
// Created by Administrator on 2025/5/17.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_BASE_HANDLER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_BASE_HANDLER_H_

#include <BAAS.h>
#include <core_defines.h>

#include "module/auto_fight/auto_fight_d.h"

BAAS_NAMESPACE_BEGIN

class base_handler {

public:

    enum ACTION_TYPE {
        AUTO,
        ACC,
        SKILL,
        RESTART
    };

    base_handler (
            BAAS* baas,
            auto_fight_d* data,
            const BAASConfig& config
    );

    static inline ACTION_TYPE act_type_st_to_enum(const std::string& act_type) {
        auto it = act_type_map.find(act_type);
        if (it != act_type_map.end()) return it->second;
        else throw ValueError("Invalid action type.");
    }

    virtual bool execute();

    virtual void display();

protected:

    BAAS* baas;

    BAASLogger* logger;

    auto_fight_d* data;

    BAASConfig config;

    std::string desc;

    static const std::map<std::string, base_handler::ACTION_TYPE> act_type_map;

    static const std::vector<std::string> act_type_st_list;

};




BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_BASE_HANDLER_H_
