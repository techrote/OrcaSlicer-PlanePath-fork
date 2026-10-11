#include "ScriptRuntime.hpp"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>
#include <thread>
#include <vector>
using namespace Slic3r::PlanePath;
static int cases = 0;
static void check(bool ok, const char *reason)
{
    ++cases;
    if (!ok) {
        std::fprintf(stderr, "PP-005 failed case %d: %s\n", cases, reason);
        std::exit(1);
    }
}
static ScriptContext context()
{
    ScriptContext c;
    c.max_x = 10000; c.max_y = 10000;
    c.parameters = {
        {"flag", ScriptParameter::Type::Boolean, 0, true, {}},
        {"step", ScriptParameter::Type::Number, 2, false, {}}
    };
    return c;
}
static ScriptResult run(std::string_view source, ScriptLimits limits = {},
                        const std::atomic<bool> *cancel = nullptr)
{
    ScriptRuntime r;
    const auto c = context();
    return r.execute(source, c, limits, cancel);
}
int main()
{
    constexpr const char *good =
        "function generate(ctx,out) assert(type(ctx.min_x)=='number' and ctx.params.flag)"
        " for i=1,32 do out:point(i*ctx.params.step,math.floor(i/2)) end end";
    auto a = run(good);
    check(a.ok(), "valid invocation");
    check(a.points.size() == 32 && a.points.front().x == 2 && a.points.back().x == 64,
          "emitted coordinates");
    check(a.statistics.vm_live_blocks_after_close == 0, "VM state freed");
    check(a.statistics.vm_peak_retained_bytes > 0, "retained memory counted");
    check(a.statistics.output_peak_reserved_bytes >= 32*sizeof(ScriptPoint),
          "host output capacity counted");
    for (int i=0; i<5; ++i) {
        auto b = run(good);
        check(b.ok() && b.points.size()==a.points.size(), "repeat size");
        for (std::size_t j=0; j<b.points.size(); ++j)
            check(b.points[j].x==a.points[j].x && b.points[j].y==a.points[j].y,
                  "repeat coordinates");
    }
    constexpr const char *deny =
        "function generate(ctx,out)"
        " assert(io==nil and os==nil and package==nil and require==nil)"
        " assert(debug==nil and load==nil and loadfile==nil and dofile==nil)"
        " assert(coroutine==nil and pcall==nil and xpcall==nil)"
        " assert(setmetatable==nil and getmetatable==nil and rawset==nil)"
        " assert(math.random==nil and math.randomseed==nil and _G==nil)"
        " out:point(0,0) out:point(1,1) end";
    check(run(deny).ok(), "forbidden ambient and indirect Lua capabilities absent");
    check(run("function generate(ctx,out) ctx.min_x=5 end").diagnostic.error ==
          ScriptError::Sandbox, "immutable context");
    check(run("function generate(ctx,out) ctx.params.step=100 end").diagnostic.error ==
          ScriptError::Sandbox, "immutable parameter proxy");
    check(run("function generate(ctx,out) out.point=0 end").diagnostic.error ==
          ScriptError::Sandbox, "immutable output proxy");
    check(run("function generate(ctx,out) out:point(0/0,0) end").diagnostic.error ==
          ScriptError::Coordinate, "NaN rejected");
    check(run("function generate(ctx,out) out:point(20000,0) end").diagnostic.error ==
          ScriptError::Coordinate, "out-of-bounds rejected");
    check(run("function generate(ctx,out) out:point(1,1) end").diagnostic.error ==
          ScriptError::Coordinate, "one-point path rejected");
    check(run("").diagnostic.error == ScriptError::InvalidSource, "empty source");
    check(run(std::string("\x1bLua",4)).diagnostic.error == ScriptError::InvalidSource,
          "binary header rejected");
    auto nul = std::string("local x=1"); nul.push_back('\0'); nul += "print(x)";
    check(run(nul).diagnostic.error == ScriptError::InvalidSource, "NUL rejected");
    check(run("function generate(").diagnostic.error == ScriptError::Syntax,
          "malformed source");
    check(run("local x=2").diagnostic.error == ScriptError::MissingGenerate,
          "missing entry point");
    {
        ScriptLimits l; l.source_bytes=10;
        check(run(good,l).diagnostic.error == ScriptError::InvalidSource, "source quota");
        l = ScriptLimits{}; l.instructions=4096; l.hook_interval=16;
        auto r = run("function generate(ctx,out) while true do end end",l);
        check(r.diagnostic.error==ScriptError::InstructionBudget, "infinite loop halted");
        check(r.statistics.instructions_charged<=l.instructions, "strict instruction accounting");
        check(r.statistics.vm_live_blocks_after_close==0, "loop VM teardown");
        check(run("function generate(ctx,out) local function f() return f() end f() end",l)
              .diagnostic.error == ScriptError::InstructionBudget, "tail recursion halted");
        l = ScriptLimits{}; l.vm_retained_bytes=96*1024; l.instructions=2000000;
        r = run("function generate(ctx,out) local a={} for i=1,50000 do a[i]={i,i,i,i} end end",l);
        check(r.diagnostic.error==ScriptError::MemoryBudget, "OOM/retained memory cap");
        check(r.statistics.vm_peak_retained_bytes<=l.vm_retained_bytes, "no VM budget overrun");
        check(r.statistics.vm_live_blocks_after_close==0, "OOM VM teardown");
        l=ScriptLimits{}; l.points=3;
        r=run("function generate(ctx,out) for i=0,20 do out:point(i,i) end end",l);
        check(r.diagnostic.error==ScriptError::OutputBudget && r.points.empty(),
              "point quota and rollback");
        l=ScriptLimits{}; l.output_bytes=sizeof(ScriptPoint)*2;
        check(run("function generate(ctx,out) for i=0,20 do out:point(i,i) end end",l)
              .diagnostic.error==ScriptError::OutputBudget, "host output bytes cap");
        l=ScriptLimits{}; l.native_work=8;
        check(run(good,l).diagnostic.error==ScriptError::NativeWorkBudget,
              "independent native helper work cap");
    }
    {
        std::atomic<bool> cancel{true};
        check(run(good,{},&cancel).diagnostic.error==ScriptError::Cancelled,
              "precancel avoids Lua state creation");
        cancel.store(false);
        ScriptLimits l; l.instructions=100000000;
        std::thread stopper([&] {
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
            cancel.store(true);
        });
        const auto r=run("function generate(ctx,out) while true do end end",l,&cancel);
        stopper.join();
        check(r.diagnostic.error==ScriptError::Cancelled, "async hook cancellation");
        check(r.statistics.vm_live_blocks_after_close==0, "cancel VM teardown");
    }
    auto err=run("function generate(ctx,out) error({danger=true}) end");
    check(err.diagnostic.error==ScriptError::Runtime && err.diagnostic.size<=255,
          "non-string error diagnostic bounded");
    err=run("function generate(ctx,out) local s='x' for i=1,13 do s=s..s end error(s) end");
    check(err.diagnostic.error==ScriptError::Runtime && err.diagnostic.size<=255,
          "large error diagnostic bounded");
    for(std::size_t i=1;i<=100;++i) {
        ScriptLimits l; l.fail_allocation_at=i;
        auto r=run(good,l);
        check(r.statistics.vm_live_blocks_after_close==0, "injected allocator fault cleanup");
        check(r.ok() || r.points.empty(), "no partial result after allocator fault");
    }
    {
        // C++ bad_alloc must unwind entirely inside the host callback, then
        // translate to a terminal Lua error without C++/Lua cross-frame unwind.
        ScriptLimits l;
        l.fail_output_reservation_at = 1;
        const auto r = run(good, l);
        check(r.diagnostic.error == ScriptError::Host,
              "injected native output allocator exception classified");
        check(r.points.empty() && !r.statistics.vm_live_blocks_after_close,
              "native allocation failure discards points and tears down VM");
        check(run(good).ok(), "native allocator fault does not taint next VM");
    }
    {
        ScriptLimits l;
        l.instructions = 1000000;
        auto r = run(
          "function generate(ctx,out) local function f(n) return 1+f(n+1) end f(0) end", l);
        check(!r.ok() && r.statistics.vm_live_blocks_after_close == 0,
              "non-tail recursive stack overflow terminates safely");
        check(run(good).ok(), "fresh state after deep recursion");
    }
    {
        // Even a large generated callback sequence must remain native-work
        // bounded, independent from VM instructions and output point quota.
        ScriptLimits l;
        l.native_work = 50;
        l.points = 100000;
        const auto r = run(
          "function generate(ctx,out) for i=1,100000 do out:point(1,1) end end", l);
        check(r.diagnostic.error == ScriptError::NativeWorkBudget,
              "native callback quota terminates tight output loop");
        check(r.points.empty(), "native-work violation never commits partial path");
    }
    check(run(good).ok(), "fresh state recovers after fault injection");
    {
        std::vector<std::thread> workers;
        std::atomic<int> passed{0};
        for(int i=0;i<8;++i)
            workers.emplace_back([&]{
                auto r=run(good);
                if(r.ok() && r.points.size()==32 && !r.statistics.vm_live_blocks_after_close)
                    ++passed;
            });
        for(auto &w:workers)w.join();
        check(passed==8, "eight independent concurrent VM executions");
    }
    {
        ScriptRuntime runtime; auto c=context();
        c.parameters[1].name="flag";
        check(runtime.execute(good,c).diagnostic.error==ScriptError::InvalidContext,
              "duplicate/out-of-order parameters");
        c=context(); c.max_x=std::numeric_limits<double>::infinity();
        check(runtime.execute(good,c).diagnostic.error==ScriptError::InvalidContext,
              "nonfinite domain validation");
    }
    std::printf("PP-005 production runtime: %d assertions passed\n",cases);
    return 0;
}
