//
// Created by pc on 2024/8/10.
//

#include "procedure/BaseProcedure.h"

#include "BAAS.h"

BAAS_NAMESPACE_BEGIN


BaseProcedure::BaseProcedure(BAAS* baas, const BAASConfig& possible_feature)
{
    this->baas = baas;
    this->possible_feature = possible_feature;
    this->logger = baas->get_logger();
    this->show_log = baas->script_show_image_compare_log;
}

void BaseProcedure::implement(
        BAASConfig& output,
        bool skip_first_screenshot
)
{
    throw std::runtime_error("BaseProcedure::implement() should not be called");
}

void BaseProcedure::clear_resource()
{
    throw std::runtime_error("BaseProcedure::clear_resource() should not be called");
}

BaseProcedure::~BaseProcedure()
{

}



BAAS_NAMESPACE_END
