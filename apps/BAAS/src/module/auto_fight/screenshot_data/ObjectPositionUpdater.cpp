//
// Created by Administrator on 2025/5/15.
//

#include "module/auto_fight/screenshot_data/ObjectPositionUpdater.h"

#include <algorithm>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "config/BAASStaticConfig.h"
#include "utils.h"
#include "utils/BAASChronoUtil.h"

BAAS_NAMESPACE_BEGIN

ObjectPositionUpdater::ObjectPositionUpdater(
        baas::BAAS* baas,
        baas::auto_fight_d* data,
        const YoloConfig& config
) : BaseDataUpdater(baas, data)
{
    logger->sub_title("AutoFight YOLO Init");
    _init_yolo_d(config);
    _init_time_cost();
    _init_yolo_model(workflow_resources().models.at(config.model));
    _warm_up_session();
}

void ObjectPositionUpdater::update()
{
    long long _t = BAASChronoUtil::getCurrentTimeMS();

    if (_t - _last_show_frames_t > 1000) {
        if (_last_log_frames > 0)
            logger->BAASInfo(std::format("[ YoloObj ] L_F : {} ({} ms)", _last_log_frames, _t - _last_show_frames_t));
        _last_log_frames = 0;
        _last_show_frames_t = _t;
    }

    if (_t - _yolo_last_update_t > _yolo_update_itv) {
        baas->get_latest_screenshot(origin_screenshot);
        _yolo->run_session(origin_screenshot, y_result, nms_op);
        _yolo_last_update_t = _t;
        auto t2 = BAASChronoUtil::getCurrentTimeMS();
        logger->BAASInfo(std::format("[ YoloObj ] Time Cost : {} ms", t2 - _t));
        ++_total_yolo_frames;
        ++_last_log_frames;
        // draw all rect
//        for (const auto& res : result.results) {
//            cv::rectangle(
//                    origin_screenshot,
//                    res.box,
//                    cv::Scalar(0, 255, 0),
//                    2
//            );
//            int center_x = res.box.x + res.box.width / 2;
//            int center_y = res.box.y + res.box.height / 2;
//             cv::putText(
//                        origin_screenshot,
//                        std::format("{}: {:.2f}", _yolo->get_classes()[res.classId], res.confidence),
//                        cv::Point(center_x, center_y),
//                        cv::FONT_HERSHEY_SIMPLEX,
//                        0.5,
//                        cv::Scalar(0, 255, 0),
//                        1
//                );
//            cv::imshow("Object Detection", origin_screenshot);
//            cv::waitKey(1);
//        }
        result.clear();
        for (const auto& _p : data->obj_last_appeared_pos) {
            std::optional<yolo_single_res> res = std::nullopt;
            for (const auto& r : y_result.results)
                if (r.class_id == _p.first && (res == std::nullopt || r.confidence > res->confidence))
                    res = r;
            if (res != std::nullopt) result[_p.first] = res;
        }
    }
}

double ObjectPositionUpdater::estimated_time_cost()
{
    return average_cost;
}

constexpr std::string ObjectPositionUpdater::data_name()
{
    return "ObjPos";
}

void ObjectPositionUpdater::display_data()
{
    if(y_result.results.empty()) {
        logger->BAASInfo("Obj  : No Object Detected.");
        return;
    }

    logger->BAASInfo(std::format(_display_format, "Object Name", "Center", "Score"));

    for (const auto& res : y_result.results) {
        std::string name = _yolo->get_classes()[res.class_id];
        int center_x = res.box.x + res.box.width / 2;
        int center_y = res.box.y + res.box.height / 2;
        std::string pos = std::format("({}, {})", center_x, center_y);
        std::string conf = std::format("{:.2f}", res.confidence);

        logger->BAASInfo(std::format(_display_format, name, pos, conf));
//        cv::rectangle(
//                origin_screenshot,
//                res.box,
//                cv::Scalar(0, 255, 0),
//                2
//        );

    }
//    cv::imshow("Object Detection", origin_screenshot);
//    cv::waitKey(1);
}

void ObjectPositionUpdater::_init_yolo_d(const YoloConfig& config)
{
    const auto& model = workflow_resources().models.at(config.model);
    data->yolo_pra.modelType = model.precision == "fp32"
            ? yolo_model_type::YOLO_DETECT_V8 : yolo_model_type::YOLO_DETECT_V8_HALF;
    switch (config.nms_type) {
        case NmsType::GROUP:
            nms_op = NMS_option::GROUP_NMS;
            break;
        case NmsType::WHOLE:
            nms_op = NMS_option::WHOLE_NMS;
            break;
        case NmsType::NONE:
            nms_op = NMS_option::NO_NMS;
            break;
    }
    _yolo_update_itv = config.update_interval;
    data->yolo_pra.model_path = BAAS_YOLO_MODEL_DIR / config.model;
    data->yolo_pra.yaml_path = BAAS_YOLO_MODEL_DIR / model.yaml_path;
    BAASUserConfig* user = baas->get_config();
    data->yolo_pra.iou_threshold = user->getFloat("/auto_fight/yolo/iou_threshold", 0.5);
    data->yolo_pra.rect_threshold = user->getFloat("/auto_fight/yolo/rect_threshold", 0.6);
    data->yolo_pra.gpu_id = user->getInt("/auto_fight/yolo/gpu_id", -1);
    data->yolo_pra.num_thread = user->getInt("/auto_fight/yolo/num_thread", 4);
    data->yolo_pra.enable_cpu_memory_arena = user->getBool("/auto_fight/yolo/enable_cpu_memory_arena", false);
}

void ObjectPositionUpdater::_init_time_cost()
{
    // gpu
    if(data->yolo_pra.gpu_id >= 0) average_cost = 2e8;
    // cpu
    else                           average_cost = 2e9 / (data->yolo_pra.num_thread * 0.5 > 2.0 ? 2.0 : data->yolo_pra.num_thread * 0.5);
}

void ObjectPositionUpdater::_init_yolo_model(const ModelResource& model)
{
    logger->BAASInfo("[ Model Init ]");
    auto s_t = BAASChronoUtil::getCurrentTimeMS();
    _yolo = std::make_unique<BAAS_Yolo_v8>();
    _yolo->init_model(data->yolo_pra, &model.classes);
    auto e_t = BAASChronoUtil::getCurrentTimeMS();
    logger->BAASInfo("Load Model T : " + std::to_string(e_t - s_t) + "ms");

    const std::vector<std::string>& cls = _yolo->get_classes();

    logger->BAASInfo("Class Size   : " + std::to_string(cls.size()));

    data->all_possible_obj_idx.clear();
    data->obj_name_to_index_map.clear();
    data->obj_last_appeared_pos.clear();

    logger->sub_title("List Formation Front Object");
    for (int i = 0; i < data->all_possible_obj_names.size(); i++) {
        logger->BAASInfo(data->all_possible_obj_names[i]);
        auto it = std::find(cls.begin(), cls.end(), data->all_possible_obj_names[i]);
        int idx_in_cls = int(std::distance(cls.begin(), it));
        data->all_possible_obj_idx.push_back(idx_in_cls);
        data->obj_name_to_index_map[data->all_possible_obj_names[i]] = idx_in_cls;
        data->obj_last_appeared_pos[idx_in_cls] = std::nullopt;
    }
}

void ObjectPositionUpdater::_warm_up_session()
{
    logger->BAASInfo("[ Session WarmUp ]");
    auto s_t = BAASChronoUtil::getCurrentTimeMS();
    _yolo->warm_up();
    auto e_t = BAASChronoUtil::getCurrentTimeMS();
    logger->BAASInfo("WarmUp T    : " + std::to_string(e_t - s_t) + "ms");
}

void ObjectPositionUpdater::write_result_into_data()
{
    for (const auto& _p : result)
        data->obj_last_appeared_pos[_p.first] = _p.second;
}

BAAS_NAMESPACE_END


