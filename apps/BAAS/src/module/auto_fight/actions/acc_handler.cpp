//
// Created by Administrator on 2025/5/17.
//

#include "module/auto_fight/actions/acc_handler.h"

#include "module/auto_fight/workflow/ActionConfig.h"

BAAS_NAMESPACE_BEGIN

acc_handler::acc_handler(
        BAAS* baas,
        auto_fight_d* data,
        const ActionStepConfig& config
) : base_handler(baas, data, config.desc),
    _op(config.acc_op)
{
}

bool acc_handler::execute()
{
    switch (_op) {
        case AccOp::PHASE_1 : {
            baas->solve_procedure("fight_set_acc_phase_1", true);
            break;
        }
        case AccOp::PHASE_2 : {
            baas->solve_procedure("fight_set_acc_phase_2", true);
            break;
        }
        case AccOp::PHASE_3 : {
            baas->solve_procedure("fight_set_acc_phase_3", true);
            break;
        }
    }

    return true;
}

void acc_handler::display() noexcept
{

}

BAAS_NAMESPACE_END


