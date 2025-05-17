//
// Created by Administrator on 2025/5/17.
//

#include "module/auto_fight/actions/acc_handler.h"

BAAS_NAMESPACE_BEGIN

const std::map<std::string, acc_handler::Op> acc_handler::op_map = {
        {"1", PHASE_1},
        {"2", PHASE_2},
        {"3", PHASE_3}
};


acc_handler::acc_handler(
        BAAS* baas,
        auto_fight_d* data,
        const BAASConfig& config
) : base_handler(baas, data, config)
{
    _parse_op();
}

bool acc_handler::execute()
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

void acc_handler::display()
{

}

void acc_handler::_parse_op()
{
    std::string op = this->config.getString("op");
    if (op_map.find(op) == op_map.end()) {
        logger->BAASError("Invalid acc_handler op: " + op);
        throw ValueError("Invalid acc_handler op.");
    }
    _op = op_map.at(op);
}


BAAS_NAMESPACE_END


