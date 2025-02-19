//
// Created by pc on 2024/8/21.
//

#include "module/mail/Mail.h"

using namespace std;

bool ISA::Mail::implement(baas::BAAS *baas)
{
    baas::BAASLogger *logger = baas->get_logger();
    baas->solve_procedure("UI-GO-TO_mail_general", true);
    if (baas->feature_appear("mail_collect-button-bright_appear")) {
        logger->sub_title("Collect Mail Reward.");
        baas->solve_procedure("COLLECT_MAIL_REWARD", true);
    } else {
        logger->BAASInfo("Mail Reward No Need to Collect.");
    }
    return true;
}
