#ifndef slic3r_FillConcentric_hpp_
#define slic3r_FillConcentric_hpp_

#include "FillBase.hpp"

namespace Slic3r {

class FillConcentric : public Fill
{
public:
    ~FillConcentric() override = default;
    bool is_self_crossing() override { return native_fill_pattern_traits(ipConcentric).self_crossing; }

protected:
    Fill* clone() const override { return new FillConcentric(*this); };
	void _fill_surface_single(
	    const FillParams                &params, 
	    unsigned int                     thickness_layers,
	    const std::pair<float, Point>   &direction, 
	    ExPolygon     		             expolygon,
	    Polylines                       &polylines_out) override;

	void _fill_surface_single(const FillParams& params,
		unsigned int                   thickness_layers,
		const std::pair<float, Point>& direction,
		ExPolygon                      expolygon,
		ThickPolylines& thick_polylines_out) override;

    bool no_sort() const override { return native_fill_pattern_traits(ipConcentric).collection_no_sort; }
};

} // namespace Slic3r

#endif // slic3r_FillConcentric_hpp_
