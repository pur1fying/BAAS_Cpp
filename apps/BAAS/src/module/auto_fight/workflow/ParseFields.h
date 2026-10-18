#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_PARSEFIELDS_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_PARSEFIELDS_H_

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "module/auto_fight/workflow/WorkflowParser.h"

BAAS_NAMESPACE_BEGIN

std::string child_path(const std::string& parent, const std::string& field);
bool stop_parsing(const WorkflowModel& result, WorkflowParseErrorMode mode);
bool errors_since(const WorkflowModel& result, std::size_t begin);
void error(
        WorkflowModel& result, WorkflowParseErrorMode mode,
        std::string code, std::string path, std::string message,
        nlohmann::json actual = nullptr, nlohmann::json expected = nullptr,
        std::string rule = "workflow.field"
);
void warning(
        WorkflowModel& result, std::string code,
        std::string path, std::string message
);
const nlohmann::json* find_json_object_field(
        const nlohmann::json& object,
        const std::string& name,
        bool required,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
);
const nlohmann::json* find_json_array_field(
        const nlohmann::json& object,
        const std::string& name,
        bool required,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
);
const nlohmann::json* find_json_string_field(
        const nlohmann::json& object,
        const std::string& name,
        bool required,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
);
const nlohmann::json* find_json_number_field(
        const nlohmann::json& object,
        const std::string& name,
        bool required,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
);
const nlohmann::json* find_json_unsigned_integer_field(
        const nlohmann::json& object,
        const std::string& name,
        bool required,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
);
std::optional<std::vector<std::string>> strings(
        const nlohmann::json& value, const std::string& path,
        WorkflowModel& result, WorkflowParseErrorMode mode
);
std::string json_string_or_default(
        const nlohmann::json& object, const std::string& key,
        const std::string& fallback = {}
);
// Keep the axis format's numeric narrowing; non-numeric input uses fallback.
std::int32_t json_number_to_int32_or_default(
        const nlohmann::json& value, std::int32_t fallback = 0
);
std::int64_t json_number_to_int64_or_default(
        const nlohmann::json& value, std::int64_t fallback = 0
);
double json_number_or_default(
        const nlohmann::json& object, const std::string& key, double fallback
);
bool contains(const std::vector<std::string>& names, const std::string& name);

template<class Enum, std::size_t Size>
std::optional<Enum> operation(
        const nlohmann::json& value, const std::array<std::string, Size>& names,
        const std::string& path, WorkflowModel& result, WorkflowParseErrorMode mode,
        const std::string& code = "FIELD_ENUM"
)
{
    const auto name = value.get<std::string>();
    const auto found = std::find(names.begin(), names.end(), name);
    if (found == names.end()) {
        error(result, mode, code, path, "Unknown operation.", value, names);
        return std::nullopt;
    }
    return static_cast<Enum>(found - names.begin());
}

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_PARSEFIELDS_H_
