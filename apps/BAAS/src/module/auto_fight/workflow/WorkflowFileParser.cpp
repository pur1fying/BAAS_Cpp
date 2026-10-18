#include "module/auto_fight/workflow/WorkflowFileParser.h"

#include <fstream>
#include <iterator>
#include <string>

BAAS_NAMESPACE_BEGIN

WorkflowModel parse_workflow_file(
        const std::filesystem::path& path,
        const WorkflowParseOptions& options
)
{
    const auto utf8 = path.u8string();
    const std::string name(utf8.begin(), utf8.end());
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {.Errors = {{.code = "FILE_READ_ERROR", .rule_id = "workflow.file",
                .message = "Cannot open workflow file.", .source_file = name}}};
    }
    const std::string text(std::istreambuf_iterator<char>{input}, {});
    if (input.bad()) {
        return {.Errors = {{.code = "FILE_READ_ERROR", .rule_id = "workflow.file",
                .message = "Cannot read workflow file.", .source_file = name}}};
    }
    auto result = parse_workflow(text, options);
    for (auto& diagnostic : result.Errors) {
        diagnostic.source_file = name;
    }
    return result;
}

BAAS_NAMESPACE_END
