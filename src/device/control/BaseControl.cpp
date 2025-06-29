//
// Created by pc on 2024/7/27.
//

#include "device/control/BaseControl.h"

BAAS_NAMESPACE_BEGIN

BaseControl::BaseControl(BAASConnection* connection)
{
    this->connection = connection;
    logger = connection->get_logger();
}

void BaseControl::init()
{
    throw std::runtime_error("Base control class init function should not be called.");
}

void BaseControl::click(
        int x,
        int y
)
{
    throw std::runtime_error("Base control class click function should not be called.");
}

void BaseControl::long_click(
        int x,
        int y,
        double duration
)
{
    throw std::runtime_error("Base control class long_click function should not be called.");
}

void BaseControl::swipe(
        int x1,
        int y1,
        int x2,
        int y2,
        double duration
)
{
    throw std::runtime_error("Base control class swipe function should not be called.");
}

void BaseControl::exit()
{
    throw std::runtime_error("Base control class exit function should not be called.");
}


BAAS_NAMESPACE_END
