#pragma once
// PP-003 standalone proposal, NOT an installed production header.
// No Lua, registry, scripted sentinel, or FillParams ownership change.
#include <array>
#include <cstdint>
#include <stdexcept>
namespace pp003 {
enum class Origin : std::uint8_t { NotPlanePath, Minimum, Center };
enum class Smoothing : std::uint8_t { Never, Always, MultilineOnly };
// This policy bundles the current direction preference + 4x sampling + contact repair.
enum class BridgeAnchorPolicy : std::uint8_t { Standard, TurningNative };
struct FillPatternTraits {
    Origin origin;
    bool separable;
    bool surface_center_controls;
    bool surface_fill_order;
    bool self_crossing;
    bool collection_no_sort;
    bool layer_forbids_reverse; // Layer::make_fills policy; NOT all entry points.
    Smoothing smoothing;
    bool sparse_multiline_ui; // UI capability; backend currently does not clamp it.
    bool pattern_bridge_flow;
    BridgeAnchorPolicy bridge_anchor_policy;
    constexpr bool plane_path() const { return origin != Origin::NotPlanePath; }
    constexpr bool smoothable(int multiline) const {
        return smoothing == Smoothing::Always ||
            (smoothing == Smoothing::MultilineOnly && multiline > 1);
    }
};
inline constexpr std::array<FillPatternTraits, 31> native_table {{
    {Origin::NotPlanePath, false, false, false, false, true, false, Smoothing::Never, false, false, BridgeAnchorPolicy::Standard}, // ipMonotonic
    {Origin::NotPlanePath, false, false, false, false, true, false, Smoothing::Never, false, false, BridgeAnchorPolicy::Standard}, // ipMonotonicLine
    {Origin::NotPlanePath, true, false, false, false, false, false, Smoothing::Never, true, false, BridgeAnchorPolicy::Standard}, // ipRectilinear
    {Origin::NotPlanePath, true, false, false, false, false, false, Smoothing::Never, true, false, BridgeAnchorPolicy::Standard}, // ipAlignedRectilinear
    {Origin::NotPlanePath, true, false, false, false, false, false, Smoothing::Never, false, false, BridgeAnchorPolicy::Standard}, // ipZigZag
    {Origin::NotPlanePath, true, false, false, false, false, false, Smoothing::Never, false, false, BridgeAnchorPolicy::Standard}, // ipCrossZag
    {Origin::NotPlanePath, true, false, false, false, false, false, Smoothing::Never, false, false, BridgeAnchorPolicy::Standard}, // ipLockedZag
    {Origin::NotPlanePath, false, false, false, false, false, false, Smoothing::Never, false, false, BridgeAnchorPolicy::Standard}, // ipLine
    {Origin::NotPlanePath, true, false, false, true, false, true, Smoothing::MultilineOnly, true, false, BridgeAnchorPolicy::Standard}, // ipGrid
    {Origin::NotPlanePath, true, false, false, true, false, false, Smoothing::MultilineOnly, true, false, BridgeAnchorPolicy::Standard}, // ipTriangles
    {Origin::NotPlanePath, true, false, false, true, false, false, Smoothing::MultilineOnly, true, false, BridgeAnchorPolicy::Standard}, // ipStars
    {Origin::NotPlanePath, true, false, false, true, false, false, Smoothing::MultilineOnly, true, false, BridgeAnchorPolicy::Standard}, // ipCubic
    {Origin::NotPlanePath, false, false, false, true, false, false, Smoothing::Never, true, false, BridgeAnchorPolicy::Standard}, // ipAdaptiveCubic
    {Origin::NotPlanePath, true, false, false, false, false, false, Smoothing::Never, true, false, BridgeAnchorPolicy::Standard}, // ipQuarterCubic
    {Origin::NotPlanePath, false, false, false, true, false, false, Smoothing::Never, true, false, BridgeAnchorPolicy::Standard}, // ipSupportCubic
    {Origin::NotPlanePath, false, false, false, false, false, false, Smoothing::Always, true, false, BridgeAnchorPolicy::Standard}, // ipLightning
    {Origin::NotPlanePath, false, false, false, false, false, false, Smoothing::Always, true, false, BridgeAnchorPolicy::Standard}, // ipHoneycomb
    {Origin::NotPlanePath, false, false, false, false, false, false, Smoothing::Always, true, false, BridgeAnchorPolicy::Standard}, // ip3DHoneycomb
    {Origin::NotPlanePath, true, false, false, false, false, false, Smoothing::Never, true, false, BridgeAnchorPolicy::Standard}, // ipLateralHoneycomb
    {Origin::NotPlanePath, true, false, false, false, false, false, Smoothing::Never, true, false, BridgeAnchorPolicy::Standard}, // ipLateralLattice
    {Origin::NotPlanePath, false, false, false, false, false, false, Smoothing::Always, true, false, BridgeAnchorPolicy::Standard}, // ipCrossHatch
    {Origin::NotPlanePath, false, false, false, false, false, false, Smoothing::Never, true, false, BridgeAnchorPolicy::Standard}, // ipTpmsD
    {Origin::NotPlanePath, false, false, false, false, false, false, Smoothing::Never, true, false, BridgeAnchorPolicy::Standard}, // ipTpmsFK
    {Origin::NotPlanePath, false, false, false, false, false, false, Smoothing::Never, true, false, BridgeAnchorPolicy::Standard}, // ipGyroid
    {Origin::NotPlanePath, false, false, true, false, true, false, Smoothing::Always, true, false, BridgeAnchorPolicy::Standard}, // ipConcentric
    {Origin::NotPlanePath, false, false, true, false, true, false, Smoothing::Never, false, false, BridgeAnchorPolicy::Standard}, // ipSpiralInset
    {Origin::Minimum, true, false, false, false, false, false, Smoothing::Always, true, false, BridgeAnchorPolicy::TurningNative}, // ipHilbertCurve
    {Origin::Center, true, true, true, false, false, false, Smoothing::Never, true, false, BridgeAnchorPolicy::Standard}, // ipArchimedeanChords
    {Origin::Center, true, true, true, false, false, false, Smoothing::Always, true, false, BridgeAnchorPolicy::TurningNative}, // ipOctagramSpiral
    {Origin::NotPlanePath, false, false, false, false, false, false, Smoothing::Never, false, false, BridgeAnchorPolicy::Standard}, // ipSupportBase
    {Origin::NotPlanePath, false, false, false, false, true, false, Smoothing::Never, false, false, BridgeAnchorPolicy::Standard}, // ipConcentricInternal
}};
// Production signature: native_fill_pattern_traits(InfillPattern).
// Reject invalid input rather than reproduce cached[type] out-of-bounds behavior.
inline const FillPatternTraits& native_fill_pattern_traits(int native_pattern) {
    if (native_pattern < 0 || native_pattern >= int(native_table.size()))
        throw std::out_of_range("invalid native infill pattern");
    return native_table[std::size_t(native_pattern)];
}
} // namespace pp003
