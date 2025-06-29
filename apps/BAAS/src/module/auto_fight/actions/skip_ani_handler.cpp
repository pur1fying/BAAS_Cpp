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
    _timeout = config.getDouble("timeout", 3.0);
    _patch = BAASConfig(nlohmann::json(), logger);
    _patch.insert("/max_execute_time", _timeout);
    _patch.insert("/tentative_click", nlohmann::json::array({true, 469, 194, 0.3}));
}

bool skip_ani_handler::execute() noexcept
{
    try {
        BAASConfig _out;
        baas->solve_procedure(
                "fight_try_skip_animation",
                _out,
                _patch,
                true
        );
    }
    catch (const std::exception& e) {
        return false;
    }

    baas->solve_procedure("fight_confirm_skip_animation", true);
    return true;
}


BAAS_NAMESPACE_END