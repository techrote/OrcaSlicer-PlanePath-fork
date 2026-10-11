#pragma once

// PP-005: deterministic, capability-limited Lua host. No Lua headers leak into
// the slicer ABI. Inputs are borrowed only for the duration of execute().
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace Slic3r::PlanePath {

struct ScriptPoint {
    double x = 0.0;
    double y = 0.0;
};

struct ScriptParameter {
    enum class Type : std::uint8_t { Number, Boolean, Enum };
    std::string_view name;
    Type type = Type::Number;
    double number = 0.0;
    bool boolean = false;
    std::string_view enumeration;
};

struct ScriptContext {
    double min_x = 0.0;
    double min_y = 0.0;
    double max_x = 0.0;
    double max_y = 0.0;
    double resolution = 1.0;
    std::uint32_t layer_index = 0;
    // Caller supplies ASCII-named, strictly increasing, validated parameters.
    std::vector<ScriptParameter> parameters;
};

struct ScriptLimits {
    // Conservative ceilings, adjustable only by a trusted caller. No time budget
    // is substituted for a deterministic instruction or native-work ceiling.
    std::size_t source_bytes = 64 * 1024;
    std::size_t vm_retained_bytes = 8 * 1024 * 1024;
    std::size_t output_bytes = 2 * 1024 * 1024;
    std::size_t points = 100000;
    std::size_t diagnostic_bytes = 255;
    std::uint64_t instructions = 4000000;
    std::uint64_t native_work = 300000;
    std::uint32_t hook_interval = 128;
    // Test-only deterministic allocation fault: zero disables injection.
    std::size_t fail_allocation_at = 0;
};

enum class ScriptPhase : std::uint8_t { Validate, Create, Setup, Load, Initialize, Generate, Finish, Close };
enum class ScriptError : std::uint8_t {
    None, InvalidSource, InvalidContext, Syntax, MissingGenerate, Sandbox,
    InstructionBudget, MemoryBudget, NativeWorkBudget, OutputBudget,
    Coordinate, Cancelled, Runtime, Host
};

struct ScriptDiagnostic {
    ScriptError error = ScriptError::None;
    ScriptPhase phase = ScriptPhase::Validate;
    int lua_status = 0;
    // No arbitrary script-controlled tostring/traceback, no absolute paths.
    std::array<char, 256> message{};
    std::size_t size = 0;
};

struct ScriptStatistics {
    std::size_t vm_peak_retained_bytes = 0;
    std::size_t vm_live_blocks_after_close = 0;
    std::size_t output_peak_reserved_bytes = 0;
    std::uint64_t instructions_charged = 0;
    std::uint64_t native_work_charged = 0;
    std::size_t emitted_points = 0;
};

struct ScriptResult {
    ScriptDiagnostic diagnostic;
    ScriptStatistics statistics;
    std::vector<ScriptPoint> points;
    bool ok() const noexcept { return diagnostic.error == ScriptError::None; }
};

class ScriptRuntime final {
public:
    // Each call owns a new Lua state, allocator and output vector. No VM survives.
    // cancel must remain alive until execute() returns. Cooperative native helpers
    // are bounded; arbitrary hard preemption requires process isolation.
    ScriptResult execute(std::string_view source,
                         const ScriptContext &context,
                         const ScriptLimits &limits = {},
                         const std::atomic<bool> *cancel = nullptr) const noexcept;
};

} // namespace Slic3r::PlanePath
