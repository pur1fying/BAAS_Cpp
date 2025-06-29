//
// Created by pc on 2024/5/31.
//
#include "module/auto_fight/BAASAutoFight.h"

using namespace std;
using namespace cv;

BAAS_NAMESPACE_BEGIN

bool BAASAutoFight::implement(BAAS *baas)
{
    BAASGlobalLogger->sub_title("Auto Fight");
    return true;
}


BAAS_NAMESPACE_END

