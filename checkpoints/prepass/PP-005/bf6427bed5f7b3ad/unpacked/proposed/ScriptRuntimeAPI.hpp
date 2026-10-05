#pragma once
// DESIGN DECLARATIONS ONLY. Intended: src/libslic3r/Script/ScriptRuntime.hpp.
// No Lua headers/types leak into FillParams or this public interface.
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
namespace Slic3r::PlanePath {
struct ResolvedScript; // immutable owning source + validated identity/ABI descriptor
struct AbiContext;     // validated local-grid context + canonical typed parameters
struct ExecutionBindings; // private bounded binding implementation; NOT raw Lua API
struct Limits {
    std::size_t source_bytes, lua_reserved_bytes, host_output_bytes, points;
    std::uint64_t vm_instructions, native_work_units;
    std::uint32_t hook_interval;
    std::size_t diagnostic_bytes; // <= Diagnostic::message.size()
};
enum class Phase : std::uint8_t { validate, create, setup, load, initialize, generate, finish, close };
enum class ErrorClass : std::uint8_t {
    none, manifest, sandbox, instruction_budget, memory_budget, point_budget,
    native_work_budget, cancelled, runtime, coordinate, stack, host
};
struct Diagnostic {
    ErrorClass kind{}; Phase phase{};
    int lua_status{}, loader_status{}, line{-1};
    std::array<char,512> message{};
    std::uint16_t length{};
    // ID/version/hash and relative source come from the owning resolved descriptor,
    // not an untrusted Lua error object's tostring metamethod.
};
struct Statistics {
    std::size_t lua_peak_reserved{}, host_peak_reserved{}, emitted_points{};
    std::uint64_t hooks{}, native_work{};
};
struct ExecutionResult {
    Diagnostic diagnostic; Statistics statistics;
    std::shared_ptr<const ResolvedScript> identity;
    bool ok() const noexcept { return diagnostic.kind == ErrorClass::none; }
};
class ScriptRuntime final {
public:
    // Creates AND destroys one independent Lua state. No live VM escapes.
    // Internally catches allocation/native exceptions and uses a fixed error buffer.
    // No default member/global mutable state; concurrent callers are independent.
    ExecutionResult execute(std::shared_ptr<const ResolvedScript> script,
        const AbiContext &context, const Limits &limits,
        const std::atomic<bool> &cancel, ExecutionBindings &bindings) const noexcept;
};
} // namespace Slic3r::PlanePath
