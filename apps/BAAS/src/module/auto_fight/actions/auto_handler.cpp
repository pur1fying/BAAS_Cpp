//
// Created by Administrator on 2025/5/17.
//
#include "module/auto_fight/actions/auto_handler.h"

BAAS_NAMESPACE_BEGIN

const std::map<std::string, auto_handler::Op> auto_handler::op_map = {
        {"on", ON},
        {"off", OFF},
        {"opposite", OPPOSITE}
};


auto_handler::auto_handler(
        BAAS* baas,
        auto_fight_d* data,
        const BAASConfig& config
) : base_handler(baas, data, config)
{
    _parse_op();
}

bool auto_handler::execute()
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

void auto_handler::display()
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
    if (!this->config.contains("op")) {
        logger->BAASError("If you want to do action [ auto ], you must fill [ op ] in config, "
                          "which indicates the auto state you expect.");
        _display_valid_auto_op();
        throw ValueError("[ /op ] must be specified.");
    }
    std::string op = this->config.getString("op");
    if (op_map.find(op) == op_map.end()) {
        logger->BAASError("Invalid auto op : [ " + op + " ]");
        _display_valid_auto_op();
        throw ValueError("Invalid auto op.");
    }
    _op = op_map.at(op);
}


BAAS_NAMESPACE_END


