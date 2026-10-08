// PP-005 mechanism probe. No Orca dependencies or geometry adapter.
// All callbacks which may raise a Lua error have only trivial automatic objects.
#include "lua_api.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

struct alignas(std::max_align_t) Header { std::size_t requested, capacity; };
struct Allocator {
    std::size_t limit=262144, reserved=0, live=0, peak=0, blocks=0;
    std::size_t calls=0, denials=0, fresh_tags=0, mismatches=0;
    std::size_t fail_from=0; // fault injection: deny growth from this allocation call onward
};
struct Context {
    Allocator mem;
    const char *source=nullptr; std::size_t source_size=0;
    std::uint64_t max_hooks=1000, hooks=0, work_limit=1000, work=0;
    std::atomic<std::uint64_t> observed_hooks{0};
    std::atomic<bool> cancel{false}, finished{false};
    bool work_self_cancel=false, host_inside=false;
    std::uint64_t hooks_in_c_before=0, hooks_in_c_after=0;
    int reason=0, raii_cleanups=0, load_status=-1;
};
enum Reason { none=0, fuel=1, cancelled=2, host_error=3, work_exhausted=4 };

// Tracks retained allocation capacity plus our header, not merely Lua's logical sizes.
// Shrink cannot fail: retain the original capacity and keep charging it.
static void *allocate(void *ud, void *ptr, std::size_t osize, std::size_t nsize) {
    auto &a=static_cast<Context *>(ud)->mem;
    ++a.calls;
    Header *old=ptr ? static_cast<Header *>(ptr)-1 : nullptr;
    if (old && old->requested!=osize) ++a.mismatches;
    if (!nsize) {
        if (old) {
            a.reserved-=sizeof(Header)+old->capacity; a.live-=old->requested;
            --a.blocks; std::free(old);
        }
        return nullptr;
    }
    if (old && nsize<=old->capacity) {
        a.live=a.live-old->requested+nsize; old->requested=nsize;
        return ptr;
    }
    const std::size_t prior=old ? old->capacity+sizeof(Header) : 0;
    if (nsize>std::numeric_limits<std::size_t>::max()-sizeof(Header)) {
        ++a.denials; return nullptr;
    }
    const std::size_t wanted=nsize+sizeof(Header);
    const std::size_t base=a.reserved-prior;
    if (base>a.limit || wanted>a.limit-base || (a.fail_from && a.calls>=a.fail_from)) {
        ++a.denials; return nullptr; // old pointer and both counters unchanged
    }
    const std::size_t old_requested=old ? old->requested : 0;
    void *fresh=std::realloc(old,wanted);
    if (!fresh) { ++a.denials; return nullptr; }
    auto *h=static_cast<Header *>(fresh);
    h->requested=nsize; h->capacity=nsize;
    a.reserved=base+wanted; a.live=a.live-old_requested+nsize;
    a.peak=std::max(a.peak,a.reserved);
    if (!ptr) { ++a.blocks; if(osize) ++a.fresh_tags; }
    return h+1;
}
static Context *context(lua_State *L) {
    void *ud=nullptr; lua_getallocf(L,&ud); return static_cast<Context *>(ud);
}
static int stop(lua_State *L,int reason) {
    Context *c=context(L);
    if (!c->reason) c->reason=reason;
    // No allocation or formatting on the terminal budget/error path.
    lua_pushlightuserdata(L,&c->reason);
    return lua_error(L);
}
static void hook(lua_State *L,lua_Debug *) {
    Context *c=context(L);
    ++c->hooks; c->observed_hooks.store(c->hooks,std::memory_order_relaxed);
    if(c->cancel.load(std::memory_order_relaxed)) { stop(L,cancelled); return; }
    if(c->hooks>=c->max_hooks) stop(L,fuel);
}
struct Guard { int &count; ~Guard(){++count;} };
// No Lua API calls here. Real C++ exceptions unwind here, before any lua_error.
static bool cpp_operation(Context *c) noexcept {
    try {
        Guard cleanup{c->raii_cleanups};
        std::vector<int> temporary(32,7);
        if(temporary[0]==7) throw std::runtime_error("synthetic sink failure");
        return true;
    } catch (...) { return false; }
}
static int host_fail(lua_State *L) {
    Context *c=context(L);
    const bool ok=cpp_operation(c); // all C++ temporaries destroyed before this returns
    if(!ok) return stop(L,host_error);
    return 0;
}
static int raise_value(lua_State *L) {
    lua_settop(L,1); return lua_error(L); // do not stringify arbitrary values
}
static int host_work(lua_State *L) {
    Context *c=context(L);
    c->hooks_in_c_before=c->hooks;
    for(std::uint64_t i=0;i<10000;++i) {
        if(c->work_self_cancel && i==31) c->cancel.store(true,std::memory_order_relaxed);
        if(c->cancel.load(std::memory_order_relaxed)) {
            c->hooks_in_c_after=c->hooks; return stop(L,cancelled);
        }
        if(c->work>=c->work_limit) {
            c->hooks_in_c_after=c->hooks; return stop(L,work_exhausted);
        }
        ++c->work;
    }
    c->hooks_in_c_after=c->hooks;
    lua_pushinteger(L,static_cast<lua_Integer>(c->work)); return 1;
}
static int host_reenter(lua_State *L) {
    lua_pushvalue(L,1); lua_call(L,0,0); return 0;
}
static int run_protected(lua_State *L) {
    Context *c=context(L);
    // No luaL_openlibs. Explicit, test-only capabilities (not proposed ABI names).
    lua_pushcfunction(L,host_fail); lua_setglobal(L,"host_fail");
    lua_pushcfunction(L,raise_value); lua_setglobal(L,"raise");
    lua_pushcfunction(L,host_work); lua_setglobal(L,"host_work");
    lua_pushcfunction(L,host_reenter); lua_setglobal(L,"host_reenter");
    c->load_status=luaL_loadbufferx(L,c->source,c->source_size,"@main.lua","t");
    if(c->load_status!=LUA_OK) return lua_error(L);
    lua_call(L,0,1); return 1;
}
struct StateOwner {
    lua_State *L; int &closed;
    ~StateOwner(){ if(L) { lua_close(L); ++closed; } }
};
struct Result {
    int status=-1, load_status=-1, reason=0, outer_cleanups=0, state_closes=0, host_cleanups=0;
    std::size_t remaining=0, live=0, peak=0, blocks=0, denials=0, mismatches=0, tags=0;
    std::uint64_t hooks=0, work=0, c_before=0, c_after=0;
    long long value=0; std::string diagnostic; int stack_top=-1;
};
static Result run(const std::string &source,std::size_t memory=262144,
                  std::uint64_t max_hooks=1000,bool async_cancel=false,
                  bool c_cancel=false,std::size_t fail_from=0) {
    Context c; c.mem.limit=memory; c.max_hooks=max_hooks; c.mem.fail_from=fail_from;
    c.source=source.data(); c.source_size=source.size(); c.work_self_cancel=c_cancel;
    Result r;
    {
        Guard outer{r.outer_cleanups};
#if LUA_VERSION_NUM >= 505
        lua_State *L=lua_newstate(allocate,&c,0x505005U);
#else
        lua_State *L=lua_newstate(allocate,&c);
#endif
        StateOwner owner{L,r.state_closes};
        if(L) {
            // New state guarantees LUA_MINSTACK. This nonallocating push is the only
            // operation before the first protected boundary.
            lua_pushcfunction(L,run_protected);
            lua_sethook(L,hook,LUA_MASKCOUNT,100);
            std::thread cancel_thread;
            if(async_cancel) cancel_thread=std::thread([&c]{
                while(c.observed_hooks.load(std::memory_order_relaxed)<10 && !c.finished.load(std::memory_order_relaxed))
                    std::this_thread::yield();
                c.cancel.store(true,std::memory_order_relaxed);
            });
            r.status=lua_pcall(L,0,1,0);
            c.finished.store(true,std::memory_order_relaxed);
            if(cancel_thread.joinable()) cancel_thread.join();
            r.stack_top=lua_gettop(L);
            if(r.status==LUA_OK) r.value=lua_tointegerx(L,-1,nullptr);
            else if(lua_type(L,-1)==LUA_TSTRING) {
                std::size_t length=0;
                const char *message=lua_tolstring(L,-1,&length);
                r.diagnostic.assign(message,std::min(length,std::size_t(96)));
            }
            lua_settop(L,0);
        }
    }
    r.load_status=c.load_status; r.reason=c.reason; r.host_cleanups=c.raii_cleanups; r.remaining=c.mem.reserved;
    r.live=c.mem.live; r.peak=c.mem.peak; r.blocks=c.mem.blocks; r.denials=c.mem.denials;
    r.tags=c.mem.fresh_tags; r.mismatches=c.mem.mismatches; r.hooks=c.hooks;
    r.work=c.work; r.c_before=c.hooks_in_c_before; r.c_after=c.hooks_in_c_after;
    return r;
}
static void check(bool v,const char *message) { if(!v) throw std::runtime_error(message); }
static std::string json_escape(const std::string &s) {
    std::string out;
    for(unsigned char c:s) {
        if(c=='"'||c=='\\') {out+='\\';out+=c;}
        else if(c=='\n')out+="\\n";
        else if(c=='\r')out+="\\r";
        else if(c<32)out+='?';
        else out+=c;
    }
    return out;
}
static void report(const std::string &name,const Result &r) {
    check(r.remaining==0&&r.live==0&&r.blocks==0,"allocator leak");
    check(r.mismatches==0,"osize accounting mismatch");
    check(r.outer_cleanups==1,"outer C++ destructor missed");
    check(r.diagnostic.size()<=96,"diagnostic overflow");
    std::cout << "{\"case\":\""<<name<<"\",\"pass\":true,\"status\":"<<r.status
      <<",\"load_status\":"<<r.load_status<<",\"reason\":"<<r.reason<<",\"value\":"<<r.value<<",\"hooks\":"<<r.hooks
      <<",\"work\":"<<r.work<<",\"peak_reserved\":"<<r.peak
      <<",\"remaining\":"<<r.remaining<<",\"blocks\":"<<r.blocks
      <<",\"denials\":"<<r.denials<<",\"fresh_type_tags\":"<<r.tags
      <<",\"outer_cleanups\":"<<r.outer_cleanups<<",\"state_closes\":"<<r.state_closes
      <<",\"host_cleanups\":"<<r.host_cleanups<<",\"stack_top\":"<<r.stack_top
      <<",\"c_hook_before\":"<<r.c_before<<",\"c_hook_after\":"<<r.c_after
      <<",\"diagnostic\":\""<<json_escape(r.diagnostic)<<"\"}\n";
}
static std::string read_fixture(const std::string &dir,const char *name) {
    std::ifstream file(dir+"/"+name,std::ios::binary);
    if(!file) throw std::runtime_error("fixture missing");
    return {std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};
}
static void allocator_unit() {
    Context c; c.mem.limit=256;
    void *p=allocate(&c,nullptr,4,64); check(p,"tagged allocation failed");
    std::memset(p,0x5a,64); auto before=c.mem.reserved;
    check(!allocate(&c,p,64,300),"growth should fail");
    check(c.mem.reserved==before&&c.mem.live==64,"failure changed accounting");
    check(static_cast<unsigned char *>(p)[63]==0x5a,"failure corrupted old block");
    void *q=allocate(&c,p,64,8); check(q==p,"shrink should retain block");
    check(c.mem.reserved==before&&c.mem.live==8,"shrink accounting incorrect");
    q=allocate(&c,q,8,48); check(q==p,"capacity reuse failed");
    q=allocate(&c,q,48,128); check(q,"growth failed");
    check(!allocate(&c,q,128,std::numeric_limits<std::size_t>::max()),"overflow accepted");
    allocate(&c,q,128,0); allocate(&c,nullptr,0,0);
    check(c.mem.reserved==0&&c.mem.live==0&&c.mem.blocks==0&&c.mem.mismatches==0,"unit leak");
    std::cout<<"{\"case\":\"allocator_transitions\",\"pass\":true,\"remaining\":0}\n";
}
int main(int argc,char **argv) {
 try {
    const std::string dir=argc>1?argv[1]:"fixtures";
    std::cout<<"{\"identity\":true,\"lua_version_number\":"<<lua_version(nullptr)
      <<",\"compile_lua_api\":"<<LUA_VERSION_NUM<<",\"cplusplus\":"<<__cplusplus
      <<",\"pointer_bytes\":"<<sizeof(void*)<<",\"header_bytes\":"<<sizeof(Header)<<"}\n";
    allocator_unit();
    Result r=run(read_fixture(dir,"success.lua"));check(r.status==0&&r.value==5050,"success");report("success",r);
    r=run(read_fixture(dir,"loop.lua"),262144,20);check(r.reason==fuel&&r.hooks==20,"fuel stop");report("instruction_limit",r);
    r=run(read_fixture(dir,"loop.lua"),262144,1000000000,true);check(r.reason==cancelled,"cancel stop");report("async_cancel",r);
    r=run(read_fixture(dir,"allocation.lua"),65536,1000000);check(r.status==LUA_ERRMEM&&r.denials>0&&r.peak<=65536,"memory bound");report("allocation_exhaustion",r);
    r=run(read_fixture(dir,"host_error.lua"));check(r.reason==host_error&&r.host_cleanups==1,"C++ callback cleanup");report("cpp_bridge_error",r);
    r=run(read_fixture(dir,"host_work.lua"));check(r.reason==work_exhausted&&r.work==1000&&r.c_before==r.c_after,"C work bound");report("c_work_no_bytecode_hooks",r);
    r=run(read_fixture(dir,"host_work.lua"),262144,1000,false,true);check(r.reason==cancelled&&r.work==31&&r.c_before==r.c_after,"C cancellation");report("c_work_cooperative_cancel",r);
    r=run(read_fixture(dir,"forbidden.lua"));check(r.status==0&&r.value==1,"forbidden API exposed");report("forbidden_api_absence",r);
    r=run(read_fixture(dir,"runtime_error.lua"));check(r.status==LUA_ERRRUN,"runtime error");report("runtime_error",r);
    r=run(read_fixture(dir,"large_diagnostic.lua"));check(r.status==LUA_ERRRUN&&r.diagnostic.size()==96,"bounded message");report("large_diagnostic",r);
    r=run("raise({secret=1})");check(r.status==LUA_ERRRUN&&r.diagnostic.empty(),"nonstring error conversion");report("nonstring_error",r);
    r=run("local = invalid");check(r.status==LUA_ERRRUN&&r.load_status==LUA_ERRSYNTAX&&r.hooks==0,"syntax status preservation");report("syntax_error",r);
    r=run(std::string("\x1bLua",4)+"not bytecode");check(r.status==LUA_ERRRUN&&r.load_status==LUA_ERRSYNTAX&&r.hooks==0,"text-only rejection status");report("binary_rejection",r);
    r=run(read_fixture(dir,"recursion.lua"),65536,1000000);check(r.status!=0&&r.peak<=65536,"recursion");report("non_tail_recursion",r);
    r=run(read_fixture(dir,"c_recursion.lua"),2097152,1000000);check(r.status==LUA_ERRRUN&&r.diagnostic.find("stack overflow")!=std::string::npos,"C stack bound");report("c_reentry_stack_guard",r);
    r=run(read_fixture(dir,"tail_recursion.lua"),65536,20);check(r.reason==fuel,"tail recursion");report("tail_recursion",r);
    r=run("return 1",1);check(r.status==-1&&r.state_closes==0,"startup OOM");report("startup_oom",r);
    for(std::size_t fail_at: {2U,5U,20U,50U,80U,120U}) {
        r=run(read_fixture(dir,"allocation.lua"),65536,1000000,false,false,fail_at);
        check(r.status!=0,"fault injection didn't fail");report("fail_growth_from_"+std::to_string(fail_at),r);
    }
    std::vector<Result> concurrent(8);
    std::vector<std::thread> threads;
    for(int i=0;i<8;++i) threads.emplace_back([&,i]{concurrent[i]=run("private=(private or 0)+1; return private");});
    for(auto &t:threads)t.join();
    for(int i=0;i<8;++i) {check(concurrent[i].value==1,"state bleed");report("isolated_"+std::to_string(i),concurrent[i]);}
    r=run(read_fixture(dir,"success.lua"));check(r.status==0&&r.value==5050,"recovery");report("fresh_after_failures",r);
    return 0;
 }catch(const std::exception &e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
