//
// Created by pc on 2024/8/10.
//
#include "procedure/BaseProcedure.h"

BaseProcedure::BaseProcedure(BAAS *baas) {
    this->baas = baas;
    this->logger = baas->get_logger();
}

