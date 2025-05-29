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
    if (!this->config.contains("op")) {
        logger->BAASError("If you want to do action [ acc ], you must fill [ op ] in config, "
                          "which indicates the target acc phase you expect.");
        _log_valid_op("[ Action acc ] [ /op ]", logger, op_st_list);
        throw ValueError("[ /op ] must be specified.");
    }
    std::string op = this->config.getString("op");
    auto it = op_map.find(op);
    if (it == op_map.end()) {
        logger->BAASError("Invalid acc op : [ " + op + " ]");
        _log_valid_op("[ Action acc ] [ /op ]", logger, op_st_list);
        throw ValueError("Invalid action acc op.");
    }
    _op = it->second;
}


BAAS_NAMESPACE_END


