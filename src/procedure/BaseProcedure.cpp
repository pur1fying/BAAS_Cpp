//
// Created by pc on 2024/8/10.
//
#include "procedure/BaseProcedure.h"

BAAS_NAMESPACE_BEGIN

BaseProcedure::BaseProcedure(BAASConfig *possible_feature)
{
    this->possible_feature = possible_feature;
    logger = nullptr;
    baas = nullptr;
}

void BaseProcedure::implement(
        BAAS *baas,
        BAASConfig &output
)
{
    throw std::runtime_error("BaseProcedure::implement() should not be called");
}

void BaseProcedure::clear_resource()
{
    throw std::runtime_error("BaseProcedure::clear_resource() should not be called");
}

BAAS_NAMESPACE_END
