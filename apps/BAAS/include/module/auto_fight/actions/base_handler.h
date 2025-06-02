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
        SKIP_ANIMATION,
        RESTART
    };

    base_handler (
            BAAS* baas,
            auto_fight_d* data,
            const BAASConfig& config
    );

    virtual bool execute();

    virtual void display();

    static const std::map<std::string, base_handler::ACTION_TYPE> act_type_map;

    static const std::vector<std::string> act_type_st_list;

protected:
    
    BAAS* baas;

    BAASLogger* logger;

    auto_fight_d* data;

    BAASConfig config;

    std::string desc;

};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_BASE_HANDLER_H_
