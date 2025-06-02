//
// Created by Administrator on 2025/5/17.
//
#include "module/auto_fight/actions/auto_handler.h"

#include "module/auto_fight/constants.h"

BAAS_NAMESPACE_BEGIN

auto_handler::auto_handler(
        BAAS* baas,
        auto_fight_d* data,
        const BAASConfig& config
) : base_handler(baas, data, config)
{
    _parse_op();
}

bool auto_handler::execute() noexcept
{
    switch (_op) {
        case ON : {
            baas->solve_procedure("fight_set_auto_on", true);
            break;
        }
        case OFF : {
            baas->solve_procedure("fight_set_auto_off", true);
            break;
        }
        case OPPOSITE : {
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

void auto_handler::_parse_op()
{
    auto _it = this->config.find("op");
    if (_it == this->config.end()) {
        logger->BAASError("[ Action Auto ] config must contain [ op ].");
        _log_valid_op("[ Action Auto ] [ op ]", logger, op_st_list);
        throw ValueError("[ Action Auto ] [ op ] not found.");
    }
    if (!_it->is_string()) {
        logger->BAASError("[ Action Auto ] [ op ] must be a string.");
        _log_valid_op("[ Action Auto ] [ op ]", logger, op_st_list);
        throw TypeError("Invalid [ Action Auto ] [ op ] Config Type.");
    }

    std::string _op_st = *_it;
    auto it = op_map.find(_op_st);
    if (it == op_map.end()) {
        logger->BAASError("Invalid [ Action Auto ] [ op ] : " + _op_st);
        _log_valid_op("[ Action Auto ] [ op ]", logger, op_st_list);
        throw ValueError("Invalid [ Action Auto ] [ op ].");
    }
    _op = it->second;
}


BAAS_NAMESPACE_END


