#include <catch2/catch_all.hpp>

#include "libslic3r/Script/ScriptRuntime.hpp"

#include <atomic>
#include <thread>
#include <vector>

using namespace Slic3r::PlanePath;

namespace {
ScriptContext context()
{
    ScriptContext ctx;
    ctx.max_x = 1000.0;
    ctx.max_y = 1000.0;
    ctx.resolution = 1.0;
    ctx.parameters = {{"step", ScriptParameter::Type::Number, 2.0, false, {}}};
    return ctx;
}
}

TEST_CASE("PP-005 Lua production host is deterministic and isolated", "[ScriptRuntime]")
{
    ScriptRuntime runtime;
    const auto ctx = context();
    const char *source =
        "function generate(ctx,out) for i=1,12 do "
        "out:point(i*ctx.params.step,math.floor(i/2)) end end";
    const auto a = runtime.execute(source, ctx);
    REQUIRE(a.ok());
    REQUIRE(a.points.size() == 12);
    REQUIRE(a.statistics.vm_live_blocks_after_close == 0);
    for (int i = 0; i < 4; ++i) {
        const auto b = runtime.execute(source, ctx);
        REQUIRE(b.ok());
        REQUIRE(b.points.size() == a.points.size());
        for (std::size_t j = 0; j < a.points.size(); ++j) {
            CHECK(b.points[j].x == a.points[j].x);
            CHECK(b.points[j].y == a.points[j].y);
        }
    }
}

TEST_CASE("PP-005 Lua capabilities and context are strictly restricted", "[ScriptRuntime]")
{
    ScriptRuntime runtime;
    const auto ctx = context();
    auto r = runtime.execute(
        "function generate(ctx,out) assert(io==nil and os==nil "
        "and package==nil and debug==nil and load==nil "
        "and pcall==nil and coroutine==nil) "
        "out:point(0,0) out:point(1,1) end", ctx);
    REQUIRE(r.ok());

    r = runtime.execute("function generate(ctx,out) ctx.params.step=7 end", ctx);
    CHECK(r.diagnostic.error == ScriptError::Sandbox);
    CHECK(r.points.empty());
    CHECK(r.statistics.vm_live_blocks_after_close == 0);

    r = runtime.execute("function generate(ctx,out) out:point(0/0,2) end", ctx);
    CHECK(r.diagnostic.error == ScriptError::Coordinate);
    CHECK(r.points.empty());
}

TEST_CASE("PP-005 C++ host exceptions cannot escape Lua frames", "[ScriptRuntime]")
{
    const ScriptRuntime runtime;
    ScriptLimits limits;
    limits.fail_output_reservation_at = 1;
    const auto ctx = context();
    auto result = runtime.execute(
        "function generate(ctx,out) out:point(1,1) out:point(2,2) end", ctx, limits);
    CHECK(result.diagnostic.error == ScriptError::Host);
    CHECK(result.points.empty());
    CHECK(result.statistics.vm_live_blocks_after_close == 0);

    limits = ScriptLimits{};
    result = runtime.execute(
        "function generate(ctx,out) out:point(1,1) out:point(2,2) end", ctx, limits);
    CHECK(result.ok());
    CHECK(result.points.size() == 2);
}

TEST_CASE("PP-005 Lua terminal budgets and cancellations clear output", "[ScriptRuntime]")
{
    ScriptRuntime runtime;
    const auto ctx = context();
    ScriptLimits limits;
    limits.instructions = 4096;
    limits.hook_interval = 16;
    auto r = runtime.execute("function generate(ctx,out) while true do end end", ctx, limits);
    CHECK(r.diagnostic.error == ScriptError::InstructionBudget);
    CHECK(r.statistics.instructions_charged <= limits.instructions);
    CHECK(r.statistics.vm_live_blocks_after_close == 0);

    limits = ScriptLimits{};
    limits.points = 3;
    r = runtime.execute("function generate(ctx,out) for i=1,100 do out:point(i,i) end end",
                        ctx, limits);
    CHECK(r.diagnostic.error == ScriptError::OutputBudget);
    CHECK(r.points.empty());

    std::atomic<bool> cancelled{true};
    r = runtime.execute("function generate(ctx,out) end", ctx, {}, &cancelled);
    CHECK(r.diagnostic.error == ScriptError::Cancelled);
}
