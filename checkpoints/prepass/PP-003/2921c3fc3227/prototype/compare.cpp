#include "FillPatternTraits.hpp"
#include "../fixtures/legacy_predicates.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
int main() {
    int checks=0;
    auto check=[&](bool x,const char* what,int p){ ++checks; if(!x){std::cerr<<what<<":"<<p<<"\n";std::exit(1);} };
    for(int p=0;p<legacy::ipCount;++p) {
        const auto e=static_cast<legacy::InfillPattern>(p);
        const auto &t=pp003::native_fill_pattern_traits(p);
        check(t.separable==legacy::is_separable_infill_pattern(e),"separable",p);
        check(t.surface_center_controls==legacy::center_controls(e),"surface center",p);
        check(t.surface_fill_order==legacy::surface_order(e),"surface order",p);
        check(t.sparse_multiline_ui==legacy::multiline_ui(e),"multiline",p);
        check(t.self_crossing==legacy::declared_self_crossing(e),"crossing",p);
        check(t.collection_no_sort==legacy::declared_no_sort(e),"no_sort",p);
        check((t.bridge_anchor_policy==pp003::BridgeAnchorPolicy::TurningNative)==legacy::turning(e),"anchors",p);
        check(t.layer_forbids_reverse==(e==legacy::ipGrid),"reverse",p);
        check(!t.pattern_bridge_flow,"native virtual bridge-flow audit",p);
        check(t.origin==(e==legacy::ipHilbertCurve?pp003::Origin::Minimum:
            legacy::center_controls(e)?pp003::Origin::Center:pp003::Origin::NotPlanePath),"origin",p);
        for(int n : {-1,0,1,2,3,10})
            check(t.smoothable(n)==legacy::is_smoothable_infill_pattern(e,n),"smooth",p);
        // CSV consumed by Python, not the source for the independent comparisons above.
        std::cout<<p<<","<<int(t.origin)<<","<<t.separable<<","<<t.surface_center_controls<<","<<t.surface_fill_order
         <<","<<t.self_crossing<<","<<t.collection_no_sort<<","<<t.layer_forbids_reverse<<","<<int(t.smoothing)
         <<","<<t.sparse_multiline_ui<<","<<t.pattern_bridge_flow<<","<<int(t.bridge_anchor_policy)
         <<","<<legacy::anchor_helper(e)<<","<<legacy::void_coefficient(e)<<"\n";
    }
    for(int p : {-1,31,32,100}) { bool rejected=false;try{(void)pp003::native_fill_pattern_traits(p);}catch(const std::out_of_range&){rejected=true;}
        check(rejected,"invalid enum rejected",p); }
    std::cerr<<"PASS "<<checks<<" predicate checks; sizeof(FillPatternTraits)="<<sizeof(pp003::FillPatternTraits)<<"\n";
}
