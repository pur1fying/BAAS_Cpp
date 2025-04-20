//
// Created by pc on 2024/8/13.
//
#include "module/restart/Restart.h"

using namespace std;

ISA_NAMESPACE_BEGIN

bool Restart::implement(baas::BAAS *baas)
{
    baas::BAASConnection *connection = baas->get_connection();
    baas::BAASLogger *logger = baas->get_logger();
    string package, activity;
    int pid;
    connection->current_app(package, activity, pid);

    string expected_package = connection->get_package_name();
    if (package != expected_package) {
        logger->BAASInfo(
                "App not running, Current package : [ " + package + " ], Expected package : [ " + expected_package +
                " ]"
        );
        logger->BAASInfo("Clear google store cache");
        connection->clear_cache(baas::static_config->getString("google_store_package_name"));
        logger->BAASInfo("Start app : " + expected_package);
        connection->start_self();
        baas::BAASConfig output;
        baas->solve_procedure("UI-GO-TO_restart_login", output);
        try {
            baas->solve_procedure("UI-GO-TO_main_page", output, true);
        }
        catch (const baas::GameStuckError &e) {
            logger->BAASError("Game stuck when go to [ main_page ].");
            logger->BAASError(e.what());
            logger->BAASError(
                    "Possible reason : fail to connect to game server, please check your network or acceleration tool."
            );
            return false;
        }
    }
    return true;
}

ISA_NAMESPACE_END
