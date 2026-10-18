#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_ACTIONCONFIG_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_ACTIONCONFIG_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

enum class ActionType { ACC, AUTO, RESTART, SKIP_ANIMATION, SKILL };
enum class AccOp { PHASE_1, PHASE_2, PHASE_3 };
enum class AutoOp { ON, OFF, OPPOSITE };
enum class SkillReleaseOp { AUTO, NAME, LAST_RELEASE_POSITION };
enum class SkillTargetOp {
    FIXED, YOLO_TOP_MEAN, YOLO_CENTER_MEAN, YOLO_GROUND_MEAN,
    YOLO_LEFT_MEAN, YOLO_RIGHT_MEAN
};
enum class SkillCheckOp { COST_DECREASE, NONE };

// Only fields selected by type and the skill operation are consumed.
struct ActionStepConfig {
    ActionType type = ActionType::RESTART;
    std::string desc;
    AccOp acc_op = AccOp::PHASE_1;
    AutoOp auto_op = AutoOp::OFF;
    double skip_animation_timeout = 3.0;
    SkillReleaseOp release_op = SkillReleaseOp::AUTO;
    std::string skill_id;
    int last_release_index = 0;
    SkillTargetOp target_op = SkillTargetOp::FIXED;
    int fixed_x = 0;
    int fixed_y = 0;
    std::vector<std::string> target_objects;
    SkillCheckOp check_op = SkillCheckOp::NONE;
    double check_value = 0.0;
    std::int64_t check_timeout = 5000;
};

struct ActionConfig {
    std::vector<std::optional<ActionStepConfig>> steps;
};

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_ACTIONCONFIG_H_
