//
// Created by Administrator on 2025/5/17.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_RESTART_HANDLER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_RESTART_HANDLER_H_

#include "base_handler.h"

BAAS_NAMESPACE_BEGIN

class restart_handler : public base_handler {

public:

    restart_handler(
            BAAS* baas,
            auto_fight_d* data,
            const BAASConfig& config
    );

    bool execute() override;

    void display() override;

private:

};



BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_RESTART_HANDLER_H_
