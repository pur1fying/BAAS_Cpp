//
// Created by Administrator on 2025/5/17.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_ACC_HANDLER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_ACC_HANDLER_H_

#include "base_handler.h"
#include "module/auto_fight/workflow/ActionConfig.h"

BAAS_NAMESPACE_BEGIN

class acc_handler : public base_handler {

public:

    acc_handler(
            BAAS* baas,
            auto_fight_d* data,
            const ActionStepConfig& config
    );

    bool execute() override;

    void display() noexcept override;

private:

    AccOp _op;

};


BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_ACC_HANDLER_H_
