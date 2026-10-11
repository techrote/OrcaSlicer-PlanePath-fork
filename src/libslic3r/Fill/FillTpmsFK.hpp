#ifndef slic3r_FillTpmsFK_hpp_
#define slic3r_FillTpmsFK_hpp_

#include <utility>

#include "libslic3r/libslic3r.h"
#include "FillBase.hpp"
#include "libslic3r/ExPolygon.hpp"
#include "libslic3r/Polyline.hpp"

namespace Slic3r {
class Point;

class FillTpmsFK : public Fill
{
public:
    FillTpmsFK() {}
    Fill* clone() const override { return new FillTpmsFK(*this); }

    // require bridge flow since most of this pattern hangs in air
    bool use_bridge_flow() const override { return native_fill_pattern_traits(ipTpmsFK).pattern_bridge_flow; }
  

    // Correction applied to regular infill angle to maximize printing
    // speed in default configuration (degrees)
    static constexpr float CorrectionAngle = -45.;

    void _fill_surface_single(const FillParams&              params,
                              unsigned int                   thickness_layers,
                              const std::pair<float, Point>& direction,
                              ExPolygon                      expolygon,
                              Polylines&                     polylines_out) override;

    bool is_self_crossing() override { return native_fill_pattern_traits(ipTpmsFK).self_crossing; }

};

} // namespace Slic3r

#endif // slic3r_FillTpmsFK_hpp_