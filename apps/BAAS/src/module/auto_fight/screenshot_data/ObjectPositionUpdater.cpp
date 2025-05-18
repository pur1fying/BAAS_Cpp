//
// Created by Administrator on 2025/5/15.
//

#include "module/auto_fight/screenshot_data/ObjectPositionUpdater.h"

#include <format>

#include "utils.h"

BAAS_NAMESPACE_BEGIN

ObjectPositionUpdater::ObjectPositionUpdater(
        baas::BAAS* baas,
        baas::auto_fight_d* data
) : BaseDataUpdater(baas, data) {
    logger->sub_title("AutoFight YOLO Init");
    _init_yolo_d();
    _init_time_cost();
    _init_yolo_model();
    _warm_up_session();
}

void ObjectPositionUpdater::update()
{
    long long _t = BAASUtil::getCurrentTimeMS();
    if (_t - _yolo_last_update_t > _yolo_update_itv) {
        baas->get_latest_screenshot(origin_screenshot);
        _yolo->run_session(origin_screenshot, result, nms_op);
        _yolo_last_update_t = _t;
        for (const auto& _p : data->obj_last_appeared_pos) {
            std::optional<yolo_single_res> res = std::nullopt;
            for (const auto& r : result.results)
                if (r.classId == _p.first && (res == std::nullopt || r.confidence > res->confidence))
                    res = r;
            if (res != std::nullopt) data->obj_last_appeared_pos[_p.first] = res;
        }
    }
}

double ObjectPositionUpdater::estimated_time_cost() {
    return average_cost;
}

constexpr std::string ObjectPositionUpdater::data_name() {
    return "ObjPos";
}

void ObjectPositionUpdater::display_data() {
    if(result.results.empty()) {
        logger->BAASInfo("Obj  : No Object Detected.");
        return;
    }

    logger->BAASInfo(std::format(_display_format, "Object Name", "Center", "Score"));

    for (const auto& res : result.results) {
        std::string name = _yolo->get_classes()[res.classId];
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

void ObjectPositionUpdater::_init_yolo_d() {
    logger->BAASInfo("[ Config Load ]");
    std::string model = data->d_fight.getString("/yolo_setting/model", "best.onnx");
    logger->BAASInfo("Model Name  : " + model);

    std::string _j_p = "/BAAS/auto_fight/yolo/models/" + model;
    if(!static_config->contains(_j_p)) {
        logger->BAASError("Model [ " + model + " ] not found.");
        throw ValueError("YOLO Model not found.");
    }

    std::string model_type = static_config->getString(_j_p + "/type");
         if(model_type == "fp32")   data->yolo_pra.modelType = yolo_model_type::YOLO_DETECT_V8;
    else if(model_type == "fp16")   data->yolo_pra.modelType = yolo_model_type::YOLO_DETECT_V8_HALF;
    else {
         logger->BAASError("Invalid Model Type [ " + model_type + " ].");
         throw ValueError("Unsupported Model Type.");
    }
    std::string nms_type = data->d_fight.getString("/yolo_setting/nms_type", "group");
         if(nms_type == "group")   nms_op = NMS_option::GROUP_NMS;
    else if(nms_type == "whole")   nms_op = NMS_option::WHOLE_NMS;
    else if(nms_type == "no")      nms_op = NMS_option::NO_NMS;
    else {
         logger->BAASError("Invalid NMS type [ " + nms_type + " ].");
         throw ValueError("Unsupported NMS Type.");
    }
    _yolo_update_itv = data->d_fight.getLLong("/yolo_setting/update_interval", 100);

    data->yolo_pra.model_path = BAAS_YOLO_MODEL_DIR / model;
    data->yolo_pra.yaml_path = BAAS_YOLO_MODEL_DIR / static_config->getString(_j_p + "/yaml_path");

    BAASUserConfig* config = baas->get_config();
    data->yolo_pra.iou_threshold = config->getDouble("/auto_fight/yolo/iou_threshold", 0.5);
    data->yolo_pra.rect_threshold = config->getDouble("/auto_fight/yolo/rect_threshold", 0.6);

    data->yolo_pra.gpu_id = config->getInt("/auto_fight/yolo/gpu_id", -1);
    data->yolo_pra.num_thread = config->getInt("/auto_fight/yolo/num_thread", 4);
    data->yolo_pra.enable_cpu_memory_arena = config->getBool("/auto_fight/yolo/enable_cpu_memory_arena", false);

    logger->BAASInfo("GPU ID      : " + std::to_string(data->yolo_pra.gpu_id));
    logger->BAASInfo("Num Thread  : " + std::to_string(data->yolo_pra.num_thread));
    logger->BAASInfo("Memory Pool : " + std::to_string(data->yolo_pra.enable_cpu_memory_arena));

    logger->BAASInfo("Update Itv  : " + std::to_string(_yolo_update_itv) + "ms");
    logger->BAASInfo("Model Type  : " + model_type);
    logger->BAASInfo("Config Path : " + data->yolo_pra.yaml_path.string());
    logger->BAASInfo("NMS Type    : " + nms_type);
    logger->BAASInfo("IOU  T      : " + std::to_string(data->yolo_pra.iou_threshold));
    logger->BAASInfo("Rect T      : " + std::to_string(data->yolo_pra.rect_threshold));

}

void ObjectPositionUpdater::_init_time_cost()
{
    // gpu
    if(data->yolo_pra.gpu_id >= 0) average_cost = 2e8;
    // cpu
    else                           average_cost = 2e9 / min(data->yolo_pra.num_thread * 0.5, 2.0);
}

void ObjectPositionUpdater::_init_yolo_model()
{
    logger->BAASInfo("[ Model Init ]");
    auto s_t = BAASUtil::getCurrentTimeMS();
    _yolo = std::make_unique<BAAS_Yolo_v8>();
    _yolo->init_model(data->yolo_pra);
    auto e_t = BAASUtil::getCurrentTimeMS();
    logger->BAASInfo("Load  Model T: " + std::to_string(e_t - s_t) + "ms");

    const std::vector<std::string>& cls = _yolo->get_classes();

    logger->BAASInfo("Class Size   : " + std::to_string(cls.size()));

    data->all_possible_obj_names = data->d_fight.get<std::vector<std::string>>("/formation/front");

    logger->sub_title("List Formation Front Object");
    for (int i = 0; i < data->all_possible_obj_names.size(); i++) {
        logger->BAASInfo(data->all_possible_obj_names[i]);
        auto it = std::find(cls.begin(), cls.end(), data->all_possible_obj_names[i]);
        if(it == cls.end()) {
            logger->BAASError("Object Name [ " + data->all_possible_obj_names[i] + " ] not found in YOLO class list.");
            throw ValueError("Invalid YOLO Object Name.");
        }
        int idx_in_cls = int(std::distance(cls.begin(), it));
        data->all_possible_obj_idx.push_back(idx_in_cls);
        data->obj_name_to_index_map[data->all_possible_obj_names[i]] = idx_in_cls;
        data->obj_last_appeared_pos[idx_in_cls] = std::nullopt;
    }
}

void ObjectPositionUpdater::_warm_up_session()
{
    logger->BAASInfo("[ Session WarmUp ]");
    auto s_t = BAASUtil::getCurrentTimeMS();
    _yolo->warm_up();
    auto e_t = BAASUtil::getCurrentTimeMS();
    logger->BAASInfo("WarmUp T    : " + std::to_string(e_t - s_t) + "ms");
}

BAAS_NAMESPACE_END




