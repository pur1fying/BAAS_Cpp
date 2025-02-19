//
// Created by pc on 2024/8/12.
//

#include "module/collect_activity_fee/CollectActivityFee.h"

using namespace std;

bool ISA::CollectActivityFee::implement(baas::BAAS *baas)
{
    baas::BAASConfig config;
    baas->solve_procedure("COLLECT_ACTIVITY_FEE", config);
    string end = config.getString("end");
    baas::BAASLogger *logger = baas->get_logger();
    if (end == "activity_fee_unable-to-collect_appear") logger->BAASInfo("Activity fee is not able to collect.");
    else if (end == "UI-AT_activity_fee") {
        logger->BAASInfo("Activity fee collected.");
        logger->BAASInfo("UI Return to main page.");
        baas->solve_procedure("UI-FROM_activity_fee_TO_main_page", config, true);
    }
    return true;
}
