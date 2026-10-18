//
// Created by Administrator on 2025/5/17.
//
#include "module/auto_fight/actions/auto_handler.h"

#include <optional>

#include "module/auto_fight/workflow/ActionConfig.h"

BAAS_NAMESPACE_BEGIN

auto_handler::auto_handler(
        BAAS* baas,
        auto_fight_d* data,
        const ActionStepConfig& config
) : base_handler(baas, data, config.desc),
    _op(config.auto_op)
{
}

bool auto_handler::execute()
{
    switch (_op) {
        case AutoOp::ON : {
            baas->solve_procedure("fight_set_auto_on", true);
            break;
        }
        case AutoOp::OFF : {
            baas->solve_procedure("fight_set_auto_off", true);
            break;
        }
        case AutoOp::OPPOSITE : {
            std::optional<bool> _auto_state;
            _auto_state = _get_fight_auto_state();
            if (!_auto_state.has_value()) {
                logger->BAASError("auto_handler failed to detect auto state.");
                return false;
            }
            if (_auto_state.value()) baas->solve_procedure("fight_set_auto_off", true);
            else                     baas->solve_procedure("fight_set_auto_on", true);
        }
    }

    return true;
}

void auto_handler::display() noexcept
{

}

std::optional<bool> auto_handler::_get_fight_auto_state()
{
    BAASConfig _out;
    try{
        baas->solve_procedure("fight_get_auto_state", _out, true);
        if(_out.getString("end") == "fight_auto_on") return true;
        else return false;
    }
    catch (const std::exception& e) {
        return std::nullopt;
    }
}

BAAS_NAMESPACE_END


