#include "module/auto_fight/workflow/WorkflowModel.h"

#include <algorithm>

BAAS_NAMESPACE_BEGIN

bool WorkflowModel::has_errors() const
{
    return std::any_of(Errors.begin(), Errors.end(), [](const auto& item) {
        return item.severity == DiagnosticSeverity::ERROR_DIAGNOSTIC;
    });
}

BAAS_NAMESPACE_END
