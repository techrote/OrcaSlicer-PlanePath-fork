#include "ScriptRuntime.hpp"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>
#include <string_view>

namespace Slic3r::PlanePath {
namespace {

// Every Lua callback/hook which can raise Lua's C longjmp contains only trivially
// destructible automatic variables. Never raise from inside a C++ catch block.
struct alignas(std::max_align_t) Allocation {
    std::size_t capacity;
};
struct Frame {
    const ScriptContext *context;
    const ScriptLimits *limits;
    const std::atomic<bool> *cancel;
    ScriptResult *result;
    ScriptError terminal = ScriptError::None;
    std::size_t retained = 0;
    std::size_t peak_retained = 0;
    std::size_t live_blocks = 0;
    std::size_t allocation_attempts = 0;
    std::uint32_t next_hook_charge = 0;
};

Frame &frame(lua_State *L) noexcept
{
    return **static_cast<Frame **>(lua_getextraspace(L));
}

bool stopped(Frame &f) noexcept
{
    if (f.terminal != ScriptError::None)
        return true;
    if (f.cancel && f.cancel->load(std::memory_order_relaxed)) {
        f.terminal = ScriptError::Cancelled;
        return true;
    }
    return false;
}

bool charge(Frame &f, std::uint64_t units) noexcept
{
    if (stopped(f))
        return false;
    auto &used = f.result->statistics.native_work_charged;
    if (units > f.limits->native_work - used) {
        f.terminal = ScriptError::NativeWorkBudget;
        return false;
    }
    used += units;
    return true;
}

void *allocator(void *userdata, void *pointer, std::size_t /* Lua osize or type tag */,
                std::size_t requested) noexcept
{
    auto &f = *static_cast<Frame *>(userdata);
    auto *old = pointer ? static_cast<Allocation *>(pointer) - 1 : nullptr;
    if (requested == 0) {
        if (old) {
            f.retained -= old->capacity;
            --f.live_blocks;
            std::free(old);
        }
        return nullptr;
    }
    // Lua's osize for new allocations is a type tag, NOT an allocation size.
    // Own headers are the sole source of retained-capacity accounting.
    if (requested > std::numeric_limits<std::size_t>::max() - sizeof(Allocation)) {
        f.terminal = ScriptError::MemoryBudget;
        return nullptr;
    }
    const std::size_t old_capacity = old ? old->capacity : 0;
    if (old && requested <= old_capacity)
        return pointer; // Keep reserved capacity charged even on shrink.
    const std::size_t growth = requested - old_capacity;
    if (growth > f.limits->vm_retained_bytes - f.retained) {
        f.terminal = ScriptError::MemoryBudget;
        return nullptr; // Original pointer and accounting remain intact.
    }
    ++f.allocation_attempts;
    if (f.limits->fail_allocation_at &&
        f.allocation_attempts == f.limits->fail_allocation_at) {
        f.terminal = ScriptError::MemoryBudget;
        return nullptr;
    }
    void *replacement = std::realloc(old, sizeof(Allocation) + requested);
    if (!replacement) {
        f.terminal = ScriptError::MemoryBudget;
        return nullptr;
    }
    auto *block = static_cast<Allocation *>(replacement);
    block->capacity = requested;
    f.retained += growth;
    f.peak_retained = std::max(f.peak_retained, f.retained);
    if (!old)
        ++f.live_blocks;
    return block + 1;
}

int fail(lua_State *L, ScriptError reason) // Lua longjmp; trivial C frames only
{
    Frame &f = frame(L);
    if (f.terminal == ScriptError::None)
        f.terminal = reason;
    lua_pushliteral(L, "PlanePath Lua execution terminated");
    return lua_error(L);
}

void instruction_hook(lua_State *L, lua_Debug *)
{
    Frame &f = frame(L);
    if (stopped(f))
        (void)fail(L, f.terminal);
    auto &used = f.result->statistics.instructions_charged;
    const std::uint64_t amount = f.next_hook_charge;
    if (amount > f.limits->instructions - used)
        (void)fail(L, ScriptError::InstructionBudget);
    used += amount;
    if (used >= f.limits->instructions)
        (void)fail(L, ScriptError::InstructionBudget);
    const std::uint64_t remaining = f.limits->instructions - used;
    f.next_hook_charge = static_cast<std::uint32_t>(
        std::min<std::uint64_t>(f.limits->hook_interval, remaining));
    lua_sethook(L, instruction_hook, LUA_MASKCOUNT, static_cast<int>(f.next_hook_charge));
}

bool key_is(const char *s, std::size_t n, const char *expected) noexcept
{
    const std::size_t count = std::strlen(expected);
    return n == count && std::memcmp(s, expected, count) == 0;
}

int readonly_error(lua_State *L)
{
    return fail(L, ScriptError::Sandbox);
}

int parameter_index(lua_State *L)
{
    Frame &f = frame(L);
    if (!charge(f, 1))
        return fail(L, f.terminal);
    std::size_t n = 0;
    if (lua_type(L, 2) != LUA_TSTRING) {
        lua_pushnil(L);
        return 1;
    }
    const char *name = lua_tolstring(L, 2, &n);
    for (const ScriptParameter &p : f.context->parameters) {
        if (!charge(f, 1))
            return fail(L, f.terminal);
        if (p.name.size() == n && std::memcmp(p.name.data(), name, n) == 0) {
            switch (p.type) {
            case ScriptParameter::Type::Number: lua_pushnumber(L, p.number); break;
            case ScriptParameter::Type::Boolean: lua_pushboolean(L, p.boolean); break;
            case ScriptParameter::Type::Enum:
                lua_pushlstring(L, p.enumeration.data(), p.enumeration.size()); break;
            }
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

// The proxy is an *empty* table: even assignment to an existing property
// necessarily hits __newindex. Backing context is never exposed to Lua.
// Neither rawset, getmetatable, setmetatable nor debug APIs are available.
void push_proxy(lua_State *L, lua_CFunction index)
{
    lua_createtable(L, 0, 0);
    lua_createtable(L, 0, 3);
    lua_pushcfunction(L, index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, readonly_error);
    lua_setfield(L, -2, "__newindex");
    lua_pushliteral(L, "sealed PlanePath host binding");
    lua_setfield(L, -2, "__metatable");
    lua_setmetatable(L, -2);
}

int context_index(lua_State *L)
{
    Frame &f = frame(L);
    if (!charge(f, 1))
        return fail(L, f.terminal);
    if (lua_type(L, 2) != LUA_TSTRING) {
        lua_pushnil(L);
        return 1;
    }
    std::size_t n = 0;
    const char *key = lua_tolstring(L, 2, &n);
    const ScriptContext &c = *f.context;
    if (key_is(key, n, "min_x")) lua_pushnumber(L, c.min_x);
    else if (key_is(key, n, "min_y")) lua_pushnumber(L, c.min_y);
    else if (key_is(key, n, "max_x")) lua_pushnumber(L, c.max_x);
    else if (key_is(key, n, "max_y")) lua_pushnumber(L, c.max_y);
    else if (key_is(key, n, "resolution")) lua_pushnumber(L, c.resolution);
    else if (key_is(key, n, "layer_index"))
        lua_pushinteger(L, static_cast<lua_Integer>(c.layer_index));
    else if (key_is(key, n, "params")) push_proxy(L, parameter_index);
    else lua_pushnil(L);
    return 1;
}

int emit_point(lua_State *L)
{
    Frame &f = frame(L);
    if (!charge(f, 1))
        return fail(L, f.terminal);
    if (lua_gettop(L) != 3 || lua_type(L, 1) != LUA_TTABLE ||
        lua_type(L, 2) != LUA_TNUMBER || lua_type(L, 3) != LUA_TNUMBER)
        return fail(L, ScriptError::Coordinate);
    const double x = lua_tonumber(L, 2);
    const double y = lua_tonumber(L, 3);
    const ScriptContext &c = *f.context;
    // PP-005 does not forward points to Orca coord_t: the future adapter must
    // also check scaled-coordinate representability and deduplicate snapping.
    const double margin = c.resolution * 2;
    if (!std::isfinite(x) || !std::isfinite(y) ||
        x < c.min_x - margin || x > c.max_x + margin ||
        y < c.min_y - margin || y > c.max_y + margin)
        return fail(L, ScriptError::Coordinate);
    auto &points = f.result->points;
    const std::size_t cap = std::min(f.limits->points,
                                    f.limits->output_bytes / sizeof(ScriptPoint));
    if (points.size() >= cap)
        return fail(L, ScriptError::OutputBudget);
    if (points.size() == points.capacity()) {
        const std::size_t next = std::min<std::size_t>(cap,
            points.capacity() ? points.capacity() * 2 : 16);
        if (next <= points.capacity())
            return fail(L, ScriptError::OutputBudget);
        // Charge bounded vector-copy work separately from VM hook instructions.
        if (!charge(f, next))
            return fail(L, f.terminal);
        bool allocation_failed = false;
        try {
            points.reserve(next);
        } catch (...) {
            allocation_failed = true; // unwind C++ destructors before Lua error
        }
        if (allocation_failed)
            return fail(L, ScriptError::Host);
        if (points.capacity() > cap)
            return fail(L, ScriptError::OutputBudget);
        f.result->statistics.output_peak_reserved_bytes =
            std::max(f.result->statistics.output_peak_reserved_bytes,
                     points.capacity() * sizeof(ScriptPoint));
    }
    // Capacity is already reserved, so this insertion cannot reallocate.
    points.push_back({x, y});
    f.result->statistics.emitted_points = points.size();
    return 0;
}

int output_index(lua_State *L)
{
    Frame &f = frame(L);
    if (!charge(f, 1))
        return fail(L, f.terminal);
    if (lua_type(L, 2) == LUA_TSTRING) {
        std::size_t n = 0;
        const char *key = lua_tolstring(L, 2, &n);
        if (key_is(key, n, "point")) {
            lua_pushcfunction(L, emit_point);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

int base_type(lua_State *L)
{
    Frame &f = frame(L);
    if (!charge(f, 1))
        return fail(L, f.terminal);
    lua_pushstring(L, lua_typename(L, lua_type(L, 1)));
    return 1;
}

int base_assert(lua_State *L)
{
    Frame &f = frame(L);
    if (!charge(f, 1))
        return fail(L, f.terminal);
    if (lua_toboolean(L, 1))
        return lua_gettop(L);
    if (lua_gettop(L) >= 2)
        lua_pushvalue(L, 2);
    else
        lua_pushliteral(L, "assertion failed");
    return lua_error(L);
}

int base_error(lua_State *L)
{
    Frame &f = frame(L);
    if (!charge(f, 1))
        return fail(L, f.terminal);
    if (lua_gettop(L) == 0)
        lua_pushnil(L);
    return lua_error(L);
}

// Explicit deterministic math allowlist. Never call platform ambient RNG.
// Floating transcendentals are reproducible on a given build, not specified
// as bit-identical across all libm implementations/processor architectures.
int math_operation(lua_State *L)
{
    Frame &f = frame(L);
    if (!charge(f, 1))
        return fail(L, f.terminal);
    const int op = static_cast<int>(lua_tointeger(L, lua_upvalueindex(1)));
    const bool binary = op >= 7;
    if (lua_gettop(L) != (binary ? 2 : 1) ||
        lua_type(L, 1) != LUA_TNUMBER ||
        (binary && lua_type(L, 2) != LUA_TNUMBER))
        return fail(L, ScriptError::Runtime);
    const double a = lua_tonumber(L, 1);
    const double b = binary ? lua_tonumber(L, 2) : 0.0;
    if (!std::isfinite(a) || (binary && !std::isfinite(b)))
        return fail(L, ScriptError::Runtime);
    double result = 0.0;
    switch (op) {
    case 1: result = std::fabs(a); break;
    case 2: result = std::floor(a); break;
    case 3: result = std::ceil(a); break;
    case 4: result = std::sqrt(a); break;
    case 5: result = std::sin(a); break;
    case 6: result = std::cos(a); break;
    case 7: result = std::min(a, b); break;
    case 8: result = std::max(a, b); break;
    default: return fail(L, ScriptError::Host);
    }
    if (!std::isfinite(result))
        return fail(L, ScriptError::Runtime);
    lua_pushnumber(L, result);
    return 1;
}

int bootstrap(lua_State *L)
{
    // Protected with lua_pcall: all allocations from table construction and
    // registrations occur inside a Lua error frame, not unprotected C++ code.
    lua_pushcfunction(L, base_type); lua_setglobal(L, "type");
    lua_pushcfunction(L, base_assert); lua_setglobal(L, "assert");
    lua_pushcfunction(L, base_error); lua_setglobal(L, "error");

    lua_createtable(L, 0, 9);
    static const char *const names[] = {
        "", "abs", "floor", "ceil", "sqrt", "sin", "cos", "min", "max"
    };
    for (int op = 1; op <= 8; ++op) {
        lua_pushinteger(L, op);
        lua_pushcclosure(L, math_operation, 1);
        lua_setfield(L, -2, names[op]);
    }
    lua_pushnumber(L, 3.141592653589793238462643383279502884);
    lua_setfield(L, -2, "pi");
    lua_setglobal(L, "math");
    return 0;
}

int invoke_generate(lua_State *L)
{
    lua_getglobal(L, "generate");
    if (lua_type(L, -1) != LUA_TFUNCTION)
        return fail(L, ScriptError::MissingGenerate);
    push_proxy(L, context_index);
    push_proxy(L, output_index);
    lua_call(L, 2, 0); // guarded by enclosing lua_pcall, no C++ RAII frames
    return 0;
}

void set_diagnostic(ScriptResult &r, ScriptError err, ScriptPhase phase,
                    int status, const char *message, std::size_t n,
                    std::size_t max_bytes) noexcept
{
    r.diagnostic.error = err;
    r.diagnostic.phase = phase;
    r.diagnostic.lua_status = status;
    const std::size_t length = std::min({n, max_bytes, r.diagnostic.message.size() - 1});
    if (message && length)
        std::memcpy(r.diagnostic.message.data(), message, length);
    r.diagnostic.message[length] = '\0';
    r.diagnostic.size = length;
}

bool valid_name(std::string_view name) noexcept
{
    if (name.empty() || name.size() > 64)
        return false;
    auto initial = [](char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
    };
    if (!initial(name[0]))
        return false;
    for (char c : name)
        if (!initial(c) && !(c >= '0' && c <= '9'))
            return false;
    return true;
}

bool valid_context(const ScriptContext &c) noexcept
{
    const double bounds[] = { c.min_x, c.min_y, c.max_x, c.max_y, c.resolution };
    for (double v : bounds)
        if (!std::isfinite(v))
            return false;
    if (c.resolution <= 0.0 || c.resolution > 1000000.0 ||
        c.min_x > c.max_x || c.min_y > c.max_y ||
        c.min_x < -100000000.0 || c.min_y < -100000000.0 ||
        c.max_x > 100000000.0 || c.max_y > 100000000.0 ||
        c.parameters.size() > 64)
        return false;
    std::string_view previous;
    for (const ScriptParameter &p : c.parameters) {
        if (!valid_name(p.name) || (!previous.empty() && p.name <= previous))
            return false;
        previous = p.name;
        if (p.type == ScriptParameter::Type::Number) {
            if (!std::isfinite(p.number) || std::fabs(p.number) > 1.e9)
                return false;
        } else if (p.type == ScriptParameter::Type::Enum) {
            if (p.enumeration.empty() || p.enumeration.size() > 128)
                return false;
            for (char ch : p.enumeration)
                if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                      (ch >= '0' && ch <= '9') || ch == '_' || ch == '-'))
                    return false;
        } else if (p.type != ScriptParameter::Type::Boolean)
            return false;
    }
    return true;
}

bool valid_limits(const ScriptLimits &l) noexcept
{
    return l.source_bytes > 0 && l.source_bytes <= 1024 * 1024 &&
           l.vm_retained_bytes >= 32 * 1024 && l.vm_retained_bytes <= 64 * 1024 * 1024 &&
           l.output_bytes >= sizeof(ScriptPoint) && l.output_bytes <= 16 * 1024 * 1024 &&
           l.points > 0 && l.points <= 500000 &&
           l.diagnostic_bytes > 0 && l.diagnostic_bytes <= 255 &&
           l.instructions > 0 && l.instructions <= 100000000 &&
           l.native_work > 0 && l.native_work <= 10000000 &&
           l.hook_interval > 0 && l.hook_interval <= 1024;
}

} // namespace

ScriptResult ScriptRuntime::execute(std::string_view source,
                                    const ScriptContext &context,
                                    const ScriptLimits &limits,
                                    const std::atomic<bool> *cancel) const noexcept
{
    ScriptResult result;
    if (!valid_limits(limits) || !valid_context(context)) {
        const char *message = "invalid runtime limits or validated script context";
        set_diagnostic(result, ScriptError::InvalidContext, ScriptPhase::Validate,
                       0, message, std::strlen(message), 255);
        return result;
    }
    if (source.empty() || source.size() > limits.source_bytes ||
        source.find('\0') != std::string_view::npos ||
        static_cast<unsigned char>(source[0]) == 0x1b) {
        const char *message = "empty, oversized, embedded-NUL or bytecode source rejected";
        set_diagnostic(result, ScriptError::InvalidSource, ScriptPhase::Validate,
                       0, message, std::strlen(message), limits.diagnostic_bytes);
        return result;
    }
    Frame f{&context, &limits, cancel, &result};
    if (stopped(f)) {
        const char *message = "cancelled before Lua state creation";
        set_diagnostic(result, f.terminal, ScriptPhase::Validate,
                       0, message, std::strlen(message), limits.diagnostic_bytes);
        return result;
    }

    lua_State *L = nullptr;
    try {
        L = lua_newstate(allocator, &f, 0x50500551u); // explicit Lua 5.5 seed
        if (!L) {
            const char *message = "Lua state creation failed within memory ceiling";
            set_diagnostic(result, f.terminal == ScriptError::None ?
                           ScriptError::MemoryBudget : f.terminal,
                           ScriptPhase::Create, LUA_ERRMEM, message,
                           std::strlen(message), limits.diagnostic_bytes);
        } else {
            *static_cast<Frame **>(lua_getextraspace(L)) = &f;
            f.next_hook_charge = static_cast<std::uint32_t>(
                std::min<std::uint64_t>(limits.instructions, limits.hook_interval));
            lua_sethook(L, instruction_hook, LUA_MASKCOUNT, static_cast<int>(f.next_hook_charge));

            // lua_pushcfunction with zero upvalues is a non-allocating light C
            // function; all remaining Lua allocations are protected by pcall.
            auto failure = [&](ScriptPhase phase, int status, ScriptError fallback) {
                std::size_t length = 0;
                const char *message = nullptr;
                if (lua_type(L, -1) == LUA_TSTRING)
                    message = lua_tolstring(L, -1, &length);
                if (!message) {
                    message = "non-string Lua error or exhausted diagnostic allocator";
                    length = std::strlen(message);
                }
                set_diagnostic(result, f.terminal == ScriptError::None ? fallback : f.terminal,
                               phase, status, message, length, limits.diagnostic_bytes);
            };

            lua_pushcfunction(L, bootstrap);
            int status = lua_pcall(L, 0, 0, 0);
            if (status != LUA_OK)
                failure(ScriptPhase::Setup, status, ScriptError::Host);
            else if (stopped(f))
                failure(ScriptPhase::Setup, LUA_OK, ScriptError::Cancelled);
            else {
                status = luaL_loadbufferx(L, source.data(), source.size(),
                                          "=planepath", "t");
                if (status != LUA_OK)
                    failure(ScriptPhase::Load, status,
                            status == LUA_ERRSYNTAX ? ScriptError::Syntax : ScriptError::Runtime);
                else if (stopped(f))
                    failure(ScriptPhase::Load, LUA_OK, ScriptError::Cancelled);
                else {
                    status = lua_pcall(L, 0, 0, 0);
                    if (status != LUA_OK)
                        failure(ScriptPhase::Initialize, status, ScriptError::Runtime);
                    else if (stopped(f))
                        failure(ScriptPhase::Initialize, LUA_OK, ScriptError::Cancelled);
                    else {
                        lua_pushcfunction(L, invoke_generate);
                        status = lua_pcall(L, 0, 0, 0);
                        if (status != LUA_OK)
                            failure(ScriptPhase::Generate, status, ScriptError::Runtime);
                        else if (stopped(f))
                            failure(ScriptPhase::Finish, LUA_OK, ScriptError::Cancelled);
                        else if (result.points.size() == 1) {
                            const char *message = "one output point is not a usable path";
                            set_diagnostic(result, ScriptError::Coordinate,
                                           ScriptPhase::Finish, 0, message,
                                           std::strlen(message), limits.diagnostic_bytes);
                        }
                    }
                }
            }
        }
    } catch (...) {
        const char *message = "host failure at Lua execution boundary";
        set_diagnostic(result, ScriptError::Host, ScriptPhase::Close, 0, message,
                       std::strlen(message), limits.diagnostic_bytes);
    }
    // No script-facing finalizers/metatable mutation or close methods exist.
    // Close the *entire* state regardless of execution outcome.
    if (L)
        lua_close(L);
    result.statistics.vm_peak_retained_bytes = f.peak_retained;
    result.statistics.vm_live_blocks_after_close = f.live_blocks;
    if (f.retained || f.live_blocks) {
        const char *message = "Lua retained allocator ownership after close";
        set_diagnostic(result, ScriptError::Host, ScriptPhase::Close, 0,
                       message, std::strlen(message), limits.diagnostic_bytes);
    }
    if (!result.ok())
        result.points.clear(); // no usable partial output on any failure
    return result;
}

} // namespace Slic3r::PlanePath
