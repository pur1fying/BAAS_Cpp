//
// Created by pc on 2024/8/12.
//

#include "module/competition/Competition.h"

using namespace std;

BAAS_NAMESPACE_BEGIN

bool Competition::implement(BAAS *baas)
{
    BAASConfig config;
    BAASLogger *logger = baas->get_logger();
    baas->solve_procedure("UI-GO-TO_main_page_competition");
    baas->solve_procedure("UI-GO-TO_competition_menu", config, true);
    string end = config.getString("end");
    if (end == "competition_start-rehearsal_appear") {
        logger->BAASInfo("Competition Season Not Start, Quit");
        return true;
    } else {
        logger->sub_title("Start Competition");
        baas->solve_procedure("COMPETITION_SOLVE", true);
    }
    return true;
}

BAAS_NAMESPACE_END
