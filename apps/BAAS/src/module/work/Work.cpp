//
// Created by pc on 2024/8/12.
//

#include "module/work/Work.h"

ISA_NAMESPACE_BEGIN

bool Work::implement(baas::BAAS *baas)
{
    baas::BAASConfig config;
    baas->solve_procedure("UI-GO-TO_work_menu", config, true);

    return true;
}

ISA_NAMESPACE_END

