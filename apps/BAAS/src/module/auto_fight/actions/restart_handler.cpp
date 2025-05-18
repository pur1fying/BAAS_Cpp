//
// Created by Administrator on 2025/5/17.
//

#include "module/auto_fight/actions/restart_handler.h"

BAAS_NAMESPACE_BEGIN

restart_handler::restart_handler(
        BAAS* baas,
        auto_fight_d* data,
        const BAASConfig& config
) : base_handler(baas, data, config)
{

}

bool restart_handler::execute()
{
    logger->hr("Fight Restart");
    baas->solve_procedure("UI-GO-TO_fight_pause_page", true);

    // TODO::fight left time check

    data->last_rel_skill_slot_idx.clear();
    baas->solve_procedure("fight_execute_restart", true);

    return true;
}

void restart_handler::display()
{

}

BAAS_NAMESPACE_END


