#include "FillPatternTraits.hpp"

#include "../PrintConfig.hpp"
#include "../libslic3r.h"

namespace Slic3r {

FillPatternTraits native_fill_pattern_traits(InfillPattern pattern)
{
    FillPatternTraits traits;

    switch (pattern) {
    case ipMonotonic:
    case ipMonotonicLine:
        traits.collection_no_sort = true;
        return traits;

    case ipRectilinear:
    case ipAlignedRectilinear:
        traits.separable        = true;
        traits.sparse_multiline = true;
        return traits;

    case ipZigZag:
    case ipCrossZag:
    case ipLockedZag:
        traits.separable = true;
        return traits;

    case ipLine:
        return traits;

    case ipGrid:
        traits.separable             = true;
        traits.self_crossing         = true;
        traits.layer_forbids_reverse = true;
        traits.smoothing_policy      = SmoothingPolicy::MultilineOnly;
        traits.sparse_multiline      = true;
        return traits;

    case ipTriangles:
    case ipStars:
    case ipCubic:
        traits.separable        = true;
        traits.self_crossing    = true;
        traits.smoothing_policy = SmoothingPolicy::MultilineOnly;
        traits.sparse_multiline = true;
        return traits;

    case ipAdaptiveCubic:
    case ipSupportCubic:
        traits.self_crossing    = true;
        traits.sparse_multiline = true;
        return traits;

    case ipQuarterCubic:
        traits.separable        = true;
        traits.sparse_multiline = true;
        return traits;

    case ipLightning:
    case ipHoneycomb:
    case ip3DHoneycomb:
        traits.smoothing_policy = SmoothingPolicy::Always;
        traits.sparse_multiline = true;
        return traits;

    case ipLateralHoneycomb:
    case ipLateralLattice:
        traits.separable        = true;
        traits.sparse_multiline = true;
        return traits;

    case ipCrossHatch:
        traits.smoothing_policy = SmoothingPolicy::Always;
        traits.sparse_multiline = true;
        return traits;

    case ipTpmsD:
    case ipTpmsFK:
    case ipGyroid:
        traits.sparse_multiline = true;
        return traits;

    case ipConcentric:
        traits.surface_fill_order  = true;
        traits.collection_no_sort  = true;
        traits.smoothing_policy    = SmoothingPolicy::Always;
        traits.sparse_multiline    = true;
        return traits;

    case ipSpiralInset:
        traits.surface_fill_order = true;
        traits.collection_no_sort = true;
        return traits;

    case ipHilbertCurve:
        traits.plane_path_origin    = PlanePathOrigin::BoundsMinimum;
        traits.source_path_ordering = SourcePathOrdering::PlanePathConditionalSurfaceOrder;
        traits.separable            = true;
        traits.smoothing_policy     = SmoothingPolicy::Always;
        traits.sparse_multiline     = true;
        traits.bridge_anchor_policy = BridgeAnchorPolicy::Turning;
        return traits;

    case ipArchimedeanChords:
        traits.plane_path_origin       = PlanePathOrigin::Centered;
        traits.source_path_ordering    = SourcePathOrdering::PlanePathConditionalSurfaceOrder;
        traits.separable               = true;
        traits.surface_center_controls = true;
        traits.surface_fill_order      = true;
        traits.sparse_multiline        = true;
        return traits;

    case ipOctagramSpiral:
        traits.plane_path_origin       = PlanePathOrigin::Centered;
        traits.source_path_ordering    = SourcePathOrdering::PlanePathConditionalSurfaceOrder;
        traits.separable               = true;
        traits.surface_center_controls = true;
        traits.surface_fill_order      = true;
        traits.smoothing_policy        = SmoothingPolicy::Always;
        traits.sparse_multiline        = true;
        traits.bridge_anchor_policy    = BridgeAnchorPolicy::Turning;
        return traits;

    case ipSupportBase:
        return traits;

    case ipConcentricInternal:
        traits.collection_no_sort = true;
        return traits;

    case ipCount:
        break;
    }

    throw Slic3r::InvalidArgument("invalid native infill pattern");
}

bool is_separable_infill_pattern(InfillPattern pattern)
{
    return native_fill_pattern_traits(pattern).separable;
}

bool is_smoothable_infill_pattern(InfillPattern pattern, int multiline)
{
    return native_fill_pattern_traits(pattern).smoothable(multiline);
}

} // namespace Slic3r
