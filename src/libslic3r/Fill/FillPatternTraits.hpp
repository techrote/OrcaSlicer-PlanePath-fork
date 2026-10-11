#ifndef slic3r_FillPatternTraits_hpp_
#define slic3r_FillPatternTraits_hpp_

#include <cstdint>

namespace Slic3r {

enum InfillPattern : int;

enum class PlanePathOrigin : std::uint8_t {
    NotApplicable,
    BoundsMinimum,
    Centered,
};

enum class SourcePathOrdering : std::uint8_t {
    PatternManaged,
    PlanePathConditionalSurfaceOrder,
};

enum class SmoothingPolicy : std::uint8_t {
    Never,
    Always,
    MultilineOnly,
};

enum class BridgeAnchorPolicy : std::uint8_t {
    Standard,
    Turning,
};

struct FillPatternTraits
{
    PlanePathOrigin    plane_path_origin      { PlanePathOrigin::NotApplicable };
    SourcePathOrdering source_path_ordering   { SourcePathOrdering::PatternManaged };
    SmoothingPolicy    smoothing_policy       { SmoothingPolicy::Never };
    BridgeAnchorPolicy bridge_anchor_policy   { BridgeAnchorPolicy::Standard };

    bool separable                  { false };
    bool surface_center_controls    { false };
    bool surface_fill_order         { false };
    bool self_crossing              { false };
    bool collection_no_sort         { false };
    bool layer_forbids_reverse      { false };
    bool sparse_multiline_ui           { false };
    bool pattern_bridge_flow        { false };

    constexpr bool is_plane_path() const
    {
        return plane_path_origin != PlanePathOrigin::NotApplicable;
    }

    constexpr bool smoothable(int multiline) const
    {
        return smoothing_policy == SmoothingPolicy::Always ||
               (smoothing_policy == SmoothingPolicy::MultilineOnly && multiline > 1);
    }
};

// Checked lookup for the complete compiled native InfillPattern domain.
// Context-dependent decisions (surface role, density, rotation templates, physical bridge state,
// and SurfaceFillOrder direction) deliberately remain at their existing call sites.
FillPatternTraits native_fill_pattern_traits(InfillPattern pattern);

} // namespace Slic3r

#endif // slic3r_FillPatternTraits_hpp_
