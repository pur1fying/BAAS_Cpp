//
// Created by pc on 2025/1/17.
//

#include "module/total_assault/TotalAssault.h"
#include "BAASAutoFight.h"

bool baas::TotalAssault::implement(BAAS *baas)
{
    BAASAutoFight autoFight = BAASAutoFight(baas);
    std::vector<std::string> skillNames = {"Akane", "Ako", "Maki", "Ui", "Mika", "Himari"};
    autoFight.setSkills(skillNames);
    autoFight.refreshSkillPosition();
    while(true){
        autoFight.enter_fight();
        autoFight.reset_all();
        autoFight.startLoop();
        autoFight.restart_fight();
    }
    return false;
}
