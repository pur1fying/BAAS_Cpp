//
// Created by pc on 2024/5/31.
//

#ifndef BAAS_CXX_REFACTOR_BAASAUTOFIGHT_H
#define BAAS_CXX_REFACTOR_BAASAUTOFIGHT_H
#include "BAASScreenshot.h"
#include <map>
#include "BAASImageUtil.h"
class BAASAutoFight {
public:


private:
    class CharacterSkill {
        public:

        private:
            std::string name;
    };
    int mode;
    std::map<std::string, CharacterSkill> skills;
    BAASScreenshot* screenshot;
    const BAASRectangle threeStudentSkillRegion = {832, 594, 1142, 670};
};
#endif //BAAS_CXX_REFACTOR_BAASAUTOFIGHT_H
