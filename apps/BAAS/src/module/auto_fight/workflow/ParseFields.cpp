#include "ParseFields.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

BAAS_NAMESPACE_BEGIN

std::string child_path(const std::string& parent, const std::string& name)
{
    std::string path = parent + "/";
    for (const auto c : name) {
        if (c == '~') {
            path += "~0";
        } else if (c == '/') {
            path += "~1";
        } else {
            path += c;
        }
    }
    return path;
}

bool stop_parsing(const WorkflowModel& result, WorkflowParseErrorMode mode)
{
    return mode == WorkflowParseErrorMode::FIRST && result.has_errors();
}

bool errors_since(const WorkflowModel& result, std::size_t begin)
{
    return std::any_of(result.Errors.begin() + begin, result.Errors.end(),
            [](const auto& item) { return item.severity == DiagnosticSeverity::ERROR_DIAGNOSTIC; });
}

void error(
        WorkflowModel& result, WorkflowParseErrorMode mode,
        std::string code, std::string path, std::string message,
        nlohmann::json actual, nlohmann::json expected, std::string rule
)
{
    if (stop_parsing(result, mode)) {
        return;
    }
    result.Errors.push_back({.code = std::move(code), .rule_id = std::move(rule),
            .path = std::move(path), .message = std::move(message),
            .actual = std::move(actual), .expected = std::move(expected)});
}

void warning(
        WorkflowModel& result, std::string code,
        std::string path, std::string message
)
{
    result.Errors.push_back({.severity = DiagnosticSeverity::WARNING_DIAGNOSTIC,
            .code = std::move(code), .rule_id = "workflow.graph",
            .path = std::move(path), .message = std::move(message)});
}

using JsonFieldTypePredicate = bool (*)(const nlohmann::json&);

const nlohmann::json* find_json_field_impl(
        const nlohmann::json& object,
        const std::string& name,
        bool required,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode,
        JsonFieldTypePredicate is_expected_type,
        const std::string& expected_type
)
{
    if (stop_parsing(result, mode) || !object.is_object()) {
        return nullptr;
    }
    auto it = object.find(name);
    if (it == object.end()) {
        if (required) {
            error(result, mode, "FIELD_REQUIRED", child_path(path, name), "Required field is missing.");
        }
        return nullptr;
    }
    if (!is_expected_type(*it)) {
        error(result, mode, "FIELD_KIND", child_path(path, name),
                "Unexpected field type.", *it, expected_type);
        return nullptr;
    }
    return &*it;
}

const nlohmann::json* find_json_object_field(
        const nlohmann::json& object,
        const std::string& name,
        bool required,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
)
{
    return find_json_field_impl(object, name, required, path, result, mode,
            [](const nlohmann::json& value) { return value.is_object(); }, "object");
}

const nlohmann::json* find_json_array_field(
        const nlohmann::json& object,
        const std::string& name,
        bool required,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
)
{
    return find_json_field_impl(object, name, required, path, result, mode,
            [](const nlohmann::json& value) { return value.is_array(); }, "array");
}

const nlohmann::json* find_json_string_field(
        const nlohmann::json& object,
        const std::string& name,
        bool required,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
)
{
    return find_json_field_impl(object, name, required, path, result, mode,
            [](const nlohmann::json& value) { return value.is_string(); }, "string");
}

const nlohmann::json* find_json_number_field(
        const nlohmann::json& object,
        const std::string& name,
        bool required,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
)
{
    return find_json_field_impl(object, name, required, path, result, mode,
            [](const nlohmann::json& value) { return value.is_number(); }, "number");
}

const nlohmann::json* find_json_unsigned_integer_field(
        const nlohmann::json& object,
        const std::string& name,
        bool required,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
)
{
    return find_json_field_impl(object, name, required, path, result, mode,
            [](const nlohmann::json& value) { return value.is_number_unsigned(); },
            "unsigned integer");
}

std::optional<std::vector<std::string>> strings(
        const nlohmann::json& value, const std::string& path,
        WorkflowModel& result, WorkflowParseErrorMode mode
)
{
    const auto begin = result.Errors.size();
    std::vector<std::string> output;
    if (!value.is_array()) {
        error(result, mode, "FIELD_KIND", path, "Expected an array of strings.", value, "array");
        return std::nullopt;
    }
    for (std::size_t i = 0; i < value.size() && !stop_parsing(result, mode); ++i) {
        if (!value[i].is_string()) {
            error(result, mode, "FIELD_KIND", child_path(path, std::to_string(i)),
                    "Expected a string.", value[i], "string");
        } else {
            output.push_back(value[i].get<std::string>());
        }
    }
    return errors_since(result, begin) ? std::nullopt :
            std::optional<std::vector<std::string>>(std::move(output));
}

std::string json_string_or_default(
        const nlohmann::json& object, const std::string& key, const std::string& fallback
)
{
    const auto it = object.find(key);
    return it != object.end() && it->is_string() ? it->get<std::string>() : fallback;
}

// Preserve the original MSVC/nlohmann narrowing, independently of host compiler.
std::int64_t json_number_to_int64_or_default(const nlohmann::json& value, std::int64_t fallback)
{
    if (!value.is_number()) {
        return fallback;
    }
    if (value.is_number_unsigned()) {
        return std::bit_cast<std::int64_t>(value.get<std::uint64_t>());
    }
    if (value.is_number_integer()) {
        return value.get<std::int64_t>();
    }
    const double number = value.get<double>();
    if (!std::isfinite(number) ||
        number < static_cast<double>(std::numeric_limits<std::int64_t>::min()) ||
        number >= -static_cast<double>(std::numeric_limits<std::int64_t>::min()))
        {
        return std::numeric_limits<std::int64_t>::min();
    }
    return static_cast<std::int64_t>(number);
}

std::int32_t json_number_to_int32_or_default(const nlohmann::json& value, std::int32_t fallback)
{
    if (!value.is_number()) {
        return fallback;
    }
    if (value.is_number_unsigned()) {
        return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(value.get<std::uint64_t>()));
    }
    if (value.is_number_integer()) {
        return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(value.get<std::int64_t>()));
    }
    const double number = value.get<double>();
    if (!std::isfinite(number) ||
        number < static_cast<double>(std::numeric_limits<std::int32_t>::min()) ||
        number >= -static_cast<double>(std::numeric_limits<std::int32_t>::min()))
        {
        return std::numeric_limits<std::int32_t>::min();
    }
    return static_cast<std::int32_t>(number);
}

double json_number_or_default(const nlohmann::json& object, const std::string& key, double fallback)
{
    auto it = object.find(key);
    return it != object.end() && it->is_number() ? it->get<double>() : fallback;
}

bool contains(const std::vector<std::string>& names, const std::string& name)
{
    return std::find(names.begin(), names.end(), name) != names.end();
}

BAAS_NAMESPACE_END
