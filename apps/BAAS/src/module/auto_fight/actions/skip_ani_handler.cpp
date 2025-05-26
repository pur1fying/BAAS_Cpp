//
// Created by Administrator on 2025/5/27.
//

#include "module/auto_fight/actions/skip_ani_handler.h"

BAAS_NAMESPACE_BEGIN

void skip_ani_handler::display() noexcept
{
    
}

skip_ani_handler::skip_ani_handler(
        BAAS* baas,
        auto_fight_d* data,
        const BAASConfig& config
) : base_handler(baas, data, config)
{

}

bool skip_ani_handler::execute() noexcept
{
    try {
        baas->solve_procedure("fight_skip_animation", true);
    }
    catch (const std::exception& e) {
        return false;
    }
    baas->solve_procedure("fight_set_auto_off", true);
    return true;
}


BAAS_NAMESPACE_END