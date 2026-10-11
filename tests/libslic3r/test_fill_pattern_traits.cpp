#include <catch2/catch_all.hpp>

#include <array>
#include <memory>
#include <set>
#include <type_traits>

#include "libslic3r/Fill/FillBase.hpp"
#include "libslic3r/Fill/SurfaceFillParams.hpp"
#include "libslic3r/Exception.hpp"
#include "libslic3r/Fill/FillPatternTraits.hpp"
#include "libslic3r/PrintConfig.hpp"

using namespace Slic3r;

namespace {

template <class T>
bool contains(const std::set<T> &values, T value)
{
    return values.find(value) != values.end();
}

} // namespace

TEST_CASE("Native fill pattern traits cover every compiled pattern", "[FillPatternTraits][PlanePathConformance]")
{
    STATIC_REQUIRE(ipCount == 32);
    STATIC_REQUIRE(ipScriptedPlanePath == 31);
    STATIC_REQUIRE(ipConcentricInternal == 30);
    STATIC_REQUIRE(std::is_trivially_copyable_v<FillParams>);

    const std::array<InfillPattern, 31> patterns{{
        ipMonotonic, ipMonotonicLine,
        ipRectilinear, ipAlignedRectilinear, ipZigZag, ipCrossZag, ipLockedZag,
        ipLine, ipGrid, ipTriangles, ipStars, ipCubic, ipAdaptiveCubic, ipQuarterCubic,
        ipSupportCubic, ipLightning, ipHoneycomb, ip3DHoneycomb, ipLateralHoneycomb,
        ipLateralLattice, ipCrossHatch, ipTpmsD, ipTpmsFK, ipGyroid, ipConcentric,
        ipSpiralInset, ipHilbertCurve, ipArchimedeanChords, ipOctagramSpiral,
        ipSupportBase, ipConcentricInternal
    }};

    const std::set<InfillPattern> separable{
        ipRectilinear, ipAlignedRectilinear, ipZigZag, ipCrossZag, ipLockedZag,
        ipGrid, ipTriangles, ipStars, ipCubic, ipQuarterCubic, ipLateralHoneycomb,
        ipLateralLattice, ipHilbertCurve, ipArchimedeanChords, ipOctagramSpiral
    };
    const std::set<InfillPattern> surface_center_controls{
        ipArchimedeanChords, ipOctagramSpiral
    };
    const std::set<InfillPattern> surface_fill_order{
        ipConcentric, ipSpiralInset, ipArchimedeanChords, ipOctagramSpiral
    };
    const std::set<InfillPattern> self_crossing{
        ipGrid, ipTriangles, ipStars, ipCubic, ipAdaptiveCubic, ipSupportCubic
    };
    const std::set<InfillPattern> collection_no_sort{
        ipMonotonic, ipMonotonicLine, ipConcentric, ipSpiralInset, ipConcentricInternal
    };
    const std::set<InfillPattern> smoothing_always{
        ipHilbertCurve, ipOctagramSpiral, ipLightning, ipHoneycomb, ip3DHoneycomb,
        ipConcentric, ipCrossHatch
    };
    const std::set<InfillPattern> smoothing_multiline{
        ipGrid, ipTriangles, ipStars, ipCubic
    };
    const std::set<InfillPattern> sparse_multiline_ui{
        ipRectilinear, ipAlignedRectilinear, ipGrid, ipTriangles, ipStars, ipCubic,
        ipAdaptiveCubic, ipQuarterCubic, ipSupportCubic, ipLightning, ipHoneycomb,
        ip3DHoneycomb, ipLateralHoneycomb, ipLateralLattice, ipCrossHatch, ipTpmsD,
        ipTpmsFK, ipGyroid, ipConcentric, ipHilbertCurve, ipArchimedeanChords,
        ipOctagramSpiral
    };
    const std::set<InfillPattern> turning_bridge_anchors{
        ipHilbertCurve, ipOctagramSpiral
    };
    const std::set<InfillPattern> plane_paths{
        ipHilbertCurve, ipArchimedeanChords, ipOctagramSpiral
    };

    std::set<int> covered_values;
    for (InfillPattern pattern : patterns) {
        CAPTURE(pattern);
        REQUIRE(covered_values.insert(int(pattern)).second);

        const FillPatternTraits traits = native_fill_pattern_traits(pattern);
        CHECK(traits.separable == contains(separable, pattern));
        CHECK(traits.surface_center_controls == contains(surface_center_controls, pattern));
        CHECK(traits.surface_fill_order == contains(surface_fill_order, pattern));
        CHECK(traits.self_crossing == contains(self_crossing, pattern));
        CHECK(traits.collection_no_sort == contains(collection_no_sort, pattern));
        CHECK(traits.layer_forbids_reverse == (pattern == ipGrid));
        CHECK(traits.sparse_multiline_ui == contains(sparse_multiline_ui, pattern));
        CHECK_FALSE(traits.pattern_bridge_flow);
        CHECK((traits.bridge_anchor_policy == BridgeAnchorPolicy::Turning) == contains(turning_bridge_anchors, pattern));
        CHECK((traits.source_path_ordering == SourcePathOrdering::PlanePathConditionalSurfaceOrder) == contains(plane_paths, pattern));

        const PlanePathOrigin expected_origin =
            pattern == ipHilbertCurve ? PlanePathOrigin::BoundsMinimum :
            (pattern == ipArchimedeanChords || pattern == ipOctagramSpiral) ? PlanePathOrigin::Centered :
            PlanePathOrigin::NotApplicable;
        CHECK(traits.plane_path_origin == expected_origin);
        CHECK(traits.is_plane_path() == contains(plane_paths, pattern));

        const SmoothingPolicy expected_smoothing =
            contains(smoothing_always, pattern) ? SmoothingPolicy::Always :
            contains(smoothing_multiline, pattern) ? SmoothingPolicy::MultilineOnly :
            SmoothingPolicy::Never;
        CHECK(traits.smoothing_policy == expected_smoothing);
        CHECK(is_smoothable_infill_pattern(pattern, 1) == (expected_smoothing == SmoothingPolicy::Always));
        CHECK(is_smoothable_infill_pattern(pattern, 2) == (expected_smoothing != SmoothingPolicy::Never));
        CHECK(is_separable_infill_pattern(pattern) == traits.separable);

        std::unique_ptr<Fill> filler(Fill::new_from_type(pattern));
        REQUIRE(filler != nullptr);
        CHECK(filler->is_self_crossing() == traits.self_crossing);
        CHECK(filler->no_sort() == traits.collection_no_sort);
        CHECK(filler->use_bridge_flow() == traits.pattern_bridge_flow);
        CHECK(Fill::use_bridge_flow(pattern) == traits.pattern_bridge_flow);
    }

    REQUIRE(covered_values.size() == size_t(ipScriptedPlanePath));
    for (int value = 0; value < int(ipScriptedPlanePath); ++value)
        CHECK(covered_values.count(value) == 1);
}

TEST_CASE("Native fill pattern trait lookup rejects invalid enum values", "[FillPatternTraits][PlanePathConformance]")
{
    CHECK_THROWS(native_fill_pattern_traits(static_cast<InfillPattern>(-1)));
    CHECK_THROWS(native_fill_pattern_traits(ipCount));
    CHECK_THROWS(native_fill_pattern_traits(static_cast<InfillPattern>(int(ipCount) + 1)));
    CHECK_THROWS(Fill::use_bridge_flow(ipCount));
}

TEST_CASE("Turning bridge-anchor policy remains Hilbert and Octagram only", "[FillPatternTraits][PlanePathConformance]")
{
    CHECK(native_fill_pattern_traits(ipHilbertCurve).bridge_anchor_policy == BridgeAnchorPolicy::Turning);
    CHECK(native_fill_pattern_traits(ipOctagramSpiral).bridge_anchor_policy == BridgeAnchorPolicy::Turning);
    CHECK(native_fill_pattern_traits(ipArchimedeanChords).bridge_anchor_policy == BridgeAnchorPolicy::Standard);
}


// PP-004: the extra compiled value is a sentinel, *not* a 32nd native descriptor.
TEST_CASE("Scripted sentinel construction and unresolved failure are explicit", "[ScriptedPlanePath][FillPatternTraits]")
{
    CHECK_THROWS_AS(native_fill_pattern_traits(ipScriptedPlanePath), InvalidArgument);
    CHECK_FALSE(is_separable_infill_pattern(ipScriptedPlanePath));
    CHECK_FALSE(is_smoothable_infill_pattern(ipScriptedPlanePath));
    CHECK_FALSE(Fill::use_bridge_flow(ipScriptedPlanePath));
    std::unique_ptr<Fill> filler(Fill::new_from_type(ipScriptedPlanePath));
    REQUIRE(filler);
    CHECK_THROWS_AS(filler->is_self_crossing(), SlicingError);
    FillParams params;
    STATIC_REQUIRE(std::is_trivially_copyable_v<FillParams>);
    CHECK_THROWS_AS(filler->fill_surface(nullptr, params), SlicingError);
    ExtrusionEntitiesPtr entities;
    CHECK_THROWS_AS(filler->fill_surface_extrusion(nullptr, params, entities), SlicingError);
    CHECK(entities.empty());
}

TEST_CASE("Canonical JSON parameter identity rejects malformed and non-finite values", "[ScriptedPlanePath]")
{
    CHECK(canonical_scripted_plane_path_params(R"({ "z": [1,2], "a": {"y":2,"x":1} })")
        == R"({"a":{"x":1,"y":2},"z":[1,2]})");
    CHECK(canonical_scripted_plane_path_params(R"({"a":{"x":1,"y":2},"z":[1,2]})")
        == canonical_scripted_plane_path_params(R"({"z":[1,2],"a":{"y":2,"x":1}})"));
    CHECK_THROWS_AS(canonical_scripted_plane_path_params("{bad"), std::invalid_argument);
    CHECK_THROWS_AS(canonical_scripted_plane_path_params(R"({"a":Infinity})"), std::invalid_argument);
    CHECK_THROWS_AS(canonical_scripted_plane_path_params(R"({"a":NaN})"), std::invalid_argument);
    CHECK_THROWS_AS(canonical_scripted_plane_path_params("[1,2]"), std::invalid_argument);
    CHECK_THROWS_AS(canonical_scripted_plane_path_params(std::string(65537, 'a')), std::invalid_argument);
}

TEST_CASE("SurfaceFillParams ordering and equality both include exact scripted identity", "[ScriptedPlanePath]")
{
    PrintRegionConfig cfg;
    cfg.top_surface_script_pattern.value = "org.example.moore";
    cfg.top_surface_script_params.value = R"({"b":2,"a":1})";
    cfg.bottom_surface_script_pattern.value = "org.example.peano";
    cfg.bottom_surface_script_params.value = R"({"b":2,"a":1})";
    cfg.internal_solid_infill_script_pattern.value = "org.example.hilbert";
    cfg.internal_solid_infill_script_params.value = R"({"resolution":3})";

    SurfaceFillParams original;
    original.pattern = ipScriptedPlanePath;
    original.flow = Flow(0.45f, 0.2f, 0.4f);
    original.bridge = false;
    original.script_invocation = scripted_invocation_for_effective_pattern(
        cfg, original.pattern, ScriptedPlanePathContext::Top);
    SurfaceFillParams identical = original;
    CHECK(original == identical);
    CHECK_FALSE(original < identical);
    CHECK_FALSE(identical < original);

    cfg.top_surface_script_params.value = R"({"a":1, "b":2})";
    identical.script_invocation = scripted_invocation_for_effective_pattern(
        cfg, identical.pattern, ScriptedPlanePathContext::Top);
    CHECK(original == identical);
    CHECK_FALSE(original < identical);

    auto different = identical;
    different.script_invocation = scripted_invocation_for_effective_pattern(
        cfg, different.pattern, ScriptedPlanePathContext::Bottom);
    CHECK_FALSE(original == different);
    CHECK((original < different) != (different < original));

    cfg.top_surface_script_params.value = R"({"a":1,"b":3})";
    different.script_invocation = scripted_invocation_for_effective_pattern(
        cfg, different.pattern, ScriptedPlanePathContext::Top);
    CHECK_FALSE(original == different);
    CHECK((original < different) != (different < original));

    different.script_invocation = original.script_invocation;
    CHECK(original == different);

    // Full-narrow and split-narrow production branches invoke this same helper.
    auto full_narrow = original;
    set_native_effective_fill_pattern(full_narrow, ipConcentricInternal);
    CHECK(full_narrow.script_invocation.empty());
    CHECK(full_narrow.pattern == ipConcentricInternal);
    auto split_narrow = original;
    set_native_effective_fill_pattern(split_narrow, ipConcentricInternal);
    CHECK(split_narrow.script_invocation.empty());
    CHECK_FALSE(original.script_invocation.empty()); // The normal portion survives.

    // Helper-generated solid and native substitution never retain stale identity.
    set_native_effective_fill_pattern(split_narrow, ipRectilinear);
    CHECK(split_narrow.script_invocation.empty());
    CHECK(scripted_invocation_for_effective_pattern(cfg, ipRectilinear, ScriptedPlanePathContext::Top).empty());
    CHECK(scripted_invocation_for_effective_pattern(cfg, ipConcentricInternal, ScriptedPlanePathContext::InternalSolid).empty());

    cfg.top_surface_script_pattern.value.clear();
    CHECK_THROWS_AS(scripted_invocation_for_effective_pattern(
        cfg, ipScriptedPlanePath, ScriptedPlanePathContext::Top), SlicingError);
    CHECK_THROWS_AS(scripted_invocation_for_effective_pattern(
        cfg, ipScriptedPlanePath, ScriptedPlanePathContext::None), SlicingError);
}
