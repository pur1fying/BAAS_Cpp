//
// Created by Administrator on 2025/5/15.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_OBJECTPOSITIONUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_OBJECTPOSITIONUPDATER_H_

#include "BaseDataUpdater.h"

#include <yolo/BAAS_yolo.h>

BAAS_NAMESPACE_BEGIN

class ObjectPositionUpdater : public BaseDataUpdater {

public:

    explicit ObjectPositionUpdater(
            BAAS* baas,
            auto_fight_d* data
    );

    void update() override;

    double estimated_time_cost() override;

    constexpr std::string data_name() override;

    void display_data() override;

private:

    void _init_time_cost();

    void _init_yolo_d();

    void _init_yolo_model();

    void _warm_up_session();

    double average_cost;

    cv::Mat origin_screenshot;

    NMS_option nms_op;
    yolo_res result;

    std::unique_ptr<BAAS_Yolo_v8> _yolo;

    static constexpr auto _display_format = "| {:>15}| {:>12}| {:>5}";

};


BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_OBJECTPOSITIONUPDATER_H_
