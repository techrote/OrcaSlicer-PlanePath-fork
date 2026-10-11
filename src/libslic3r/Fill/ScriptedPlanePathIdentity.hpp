#pragma once

// PP-004: identity only; no Lua/registry, no invented package-specific traits.
#include "../PrintConfig.hpp"
#include "../Exception.hpp"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace Slic3r {

struct ScriptedPlanePathInvocation {
    std::string package_id;
    std::string canonical_params;

    bool operator==(const ScriptedPlanePathInvocation &rhs) const {
        return package_id == rhs.package_id && canonical_params == rhs.canonical_params;
    }
    bool operator<(const ScriptedPlanePathInvocation &rhs) const {
        return std::tie(package_id, canonical_params) < std::tie(rhs.package_id, rhs.canonical_params);
    }
    bool empty() const { return package_id.empty() && canonical_params.empty(); }
};

// nlohmann::json sorts object keys lexically. Input/order/whitespace do not affect
// invocation identity; JSON parameters are objects, not arbitrary JSON fragments.
// Bounds prevent unbounded script configuration from being copied in hot fill grouping.
inline std::string canonical_scripted_plane_path_params(const std::string &raw)
{
    constexpr size_t limit = 64 * 1024;
    if (raw.size() > limit)
        throw std::invalid_argument("JSON parameters exceed 64 KiB");
    try {
        const nlohmann::json value = nlohmann::json::parse(raw);
        if (!value.is_object())
            throw std::invalid_argument("Script parameters must be a JSON object");
        const std::string canonical = value.dump();
        if (canonical.size() > limit)
            throw std::invalid_argument("Canonical JSON parameters exceed 64 KiB");
        return canonical;
    } catch (const nlohmann::json::exception &e) {
        throw std::invalid_argument(std::string("Malformed JSON parameters: ") + e.what());
    }
}

enum class ScriptedPlanePathContext { None, Top, Bottom, InternalSolid };

// Only the *effective* sentinel in an intended solid context loads companion state.
// Native substitutions leave an empty identity, and malformed active input throws.
inline ScriptedPlanePathInvocation scripted_invocation_for_effective_pattern(
    const PrintRegionConfig &config, InfillPattern effective, ScriptedPlanePathContext context)
{
    if (effective != ipScriptedPlanePath) return {};
    const ConfigOptionString *id = nullptr;
    const ConfigOptionString *params = nullptr;
    switch (context) {
    case ScriptedPlanePathContext::Top:
        id = &config.top_surface_script_pattern; params = &config.top_surface_script_params; break;
    case ScriptedPlanePathContext::Bottom:
        id = &config.bottom_surface_script_pattern; params = &config.bottom_surface_script_params; break;
    case ScriptedPlanePathContext::InternalSolid:
        id = &config.internal_solid_infill_script_pattern; params = &config.internal_solid_infill_script_params; break;
    case ScriptedPlanePathContext::None:
        throw SlicingError("Scripted PlanePath is unsupported in this fill context");
    }
    if (id->value.empty())
        throw SlicingError("Scripted PlanePath package ID is missing");
    try {
        return {id->value, canonical_scripted_plane_path_params(params->value)};
    } catch (const std::invalid_argument &e) {
        throw SlicingError(std::string("Invalid scripted PlanePath parameters: ") + e.what());
    }
}

} // namespace Slic3r
