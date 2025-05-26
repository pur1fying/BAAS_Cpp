//
// Created by Administrator on 2025/5/27.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_SKIP_ANI_HANDLER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_SKIP_ANI_HANDLER_H_

#include "base_handler.h"

BAAS_NAMESPACE_BEGIN


class skip_ani_handler : public base_handler {

public:

    skip_ani_handler(
            BAAS* baas,
            auto_fight_d* data,
            const BAASConfig& config
    );

    bool execute() noexcept override;

    void display() noexcept override;

private:

    double _timeout;

};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_SKIP_ANI_HANDLER_H_
