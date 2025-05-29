//
// Created by Administrator on 2025/5/17.
//

#include "module/auto_fight/actions/base_handler.h"


BAAS_NAMESPACE_BEGIN

base_handler::base_handler(BAAS* baas, auto_fight_d* data, const BAASConfig& config)
{
    this->config = BAASConfig(config.get_config(), baas->get_logger());
    this->data = data;
    this->baas = baas;
    this->logger = baas->get_logger();
    this->desc = config.getString("desc");
}

bool base_handler::execute()
{
    throw RuntimeError("BaseHandler class execute should not be called.");
}

void base_handler::display()
{
    throw RuntimeError("BaseHandler class display should not be called.");
}


BAAS_NAMESPACE_END