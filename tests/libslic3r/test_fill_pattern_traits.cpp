#include <catch2/catch_all.hpp>

#include <array>
#include <memory>
#include <set>
#include <type_traits>

#include "libslic3r/Fill/FillBase.hpp"
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
    STATIC_REQUIRE(ipCount == 31);
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

    REQUIRE(covered_values.size() == size_t(ipCount));
    for (int value = 0; value < int(ipCount); ++value)
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
