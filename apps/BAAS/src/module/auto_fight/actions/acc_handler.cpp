//
// Created by Administrator on 2025/5/17.
//

#include "module/auto_fight/actions/acc_handler.h"
#include "module/auto_fight/constants.h"

BAAS_NAMESPACE_BEGIN

acc_handler::acc_handler(
        BAAS* baas,
        auto_fight_d* data,
        const BAASConfig& config
) : base_handler(baas, data, config)
{
    _parse_op();
}

bool acc_handler::execute() noexcept
{
    switch (_op) {
        case PHASE_1 : {
            baas->solve_procedure("fight_set_acc_phase_1", true);
            break;
        }
        case PHASE_2 : {
            baas->solve_procedure("fight_set_acc_phase_2", true);
            break;
        }
        case PHASE_3 : {
            baas->solve_procedure("fight_set_acc_phase_3", true);
            break;
        }
    }

    return true;
}

void acc_handler::display() noexcept
{

}

void acc_handler::_parse_op()
{
    auto _it = this->config.find("op");
    if (_it == this->config.end()) {
        logger->BAASError("[ Action Acc ] config must contain [ op ].");
        _log_valid_op("[ Action Acc ] [ op ]", logger, op_st_list);
        throw ValueError("[ Action Acc ] [ op ] not found.");
    }
    if (!_it->is_string()) {
        logger->BAASError("[ Action Acc ] [ op ] must be a string.");
        _log_valid_op("[ Action Acc ] [ op ]", logger, op_st_list);
        throw TypeError("Invalid [ Action Acc ] [ op ] Config Type.");
    }

    std::string _op_st = *_it;
    auto it = op_map.find(_op_st);
    if (it == op_map.end()) {
        logger->BAASError("Invalid [ Action Acc ] [ op ] : " + _op_st);
        _log_valid_op("[ Action Acc ] [ op ]", logger, op_st_list);
        throw ValueError("Invalid [ Action Acc ] [ op ].");
    }
    _op = it->second;
}


BAAS_NAMESPACE_END


