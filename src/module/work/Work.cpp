//
// Created by pc on 2024/8/12.
//

#include "module/work/Work.h"

bool ISA::Work::implement(BAAS *baas) {
    BAASConfig config;
    baas->solve_procedure("UI-GO-TO_work_menu", config, true);

    return true;
}
