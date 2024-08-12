//
// Created by pc on 2024/8/12.
//

#include "module/competition/Competition.h"

bool ISA::Competition::implement(BAAS *baas) {
    BAASConfig config;
    baas->solve_procedure("UI-GO-TO_main_page", config);
    baas->solve_procedure("UI-GO-TO_main_page_competition", config);
    baas->solve_procedure("COMPETITION_SOLVE", config);
    return true;
}
