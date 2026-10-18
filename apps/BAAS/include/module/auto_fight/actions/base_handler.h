//
// Created by Administrator on 2025/5/17.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_BASE_HANDLER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_BASE_HANDLER_H_

#include <string>

#include "BAAS.h"
#include "core_defines.h"
#include "module/auto_fight/auto_fight_d.h"

BAAS_NAMESPACE_BEGIN

class base_handler {

public:

    base_handler (
            BAAS* baas,
            auto_fight_d* data,
            std::string desc
    );

    virtual bool execute();

    virtual void display();

protected:

    BAAS* baas;

    BAASLogger* logger;

    auto_fight_d* data;

    std::string desc;

};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_BASE_HANDLER_H_
