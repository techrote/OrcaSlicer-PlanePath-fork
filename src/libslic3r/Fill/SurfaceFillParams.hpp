#pragma once

// PP-004: SurfaceFillParams is the *actual* batching key. Export it for tests,
// without modifying legacy native comparator fields or the trivially-copyable FillParams.
#include "FillBase.hpp"
#include "ScriptedPlanePathIdentity.hpp"

namespace Slic3r {

struct SurfaceFillParams
{
	// Zero based extruder ID.
    unsigned int 	extruder = 0;
	// Infill pattern, adjusted for the density etc.
    InfillPattern  	pattern = InfillPattern(0);
    ScriptedPlanePathInvocation script_invocation; // exact canonical batching identity

    // FillBase
    // in unscaled coordinates
    coordf_t    	spacing = 0.;
    // infill / perimeter overlap, in unscaled coordinates
    coordf_t    	overlap = 0.;
    // Angle as provided by the region config, in radians.
    float       	angle = 0.f;
    // Orca: fixed_angle
    bool        fixed_angle = false;
    // Is bridging used for this fill? Bridging parameters may be used even if this->flow.bridge() is not set.
    bool 			bridge;
    // Non-negative for a bridge.
    float 			bridge_angle = 0.f;

    // FillParams
    float       	density = 0.f;
    // Infill line multiplier count.
    int   multiline = 1;
    // Don't adjust spacing to fill the space evenly.
//    bool        	dont_adjust = false;
    // Length of the infill anchor along the perimeter line.
    // 1000mm is roughly the maximum length line that fits into a 32bit coord_t.
    float 			anchor_length     = 1000.f;
    float 			anchor_length_max = 1000.f;

    // width, height of extrusion, nozzle diameter, is bridge
    // For the output, for fill generator.
    Flow 			flow;

	// For the output
    ExtrusionRole	extrusion_role = ExtrusionRole(0);

	// Various print settings?

	// Index of this entry in a linear vector.
    size_t 			idx = 0;
	// Infill speed setting for the effective extrusion role.
	float role_speed = 0;

    // Params for lattice infill angles
    float lateral_lattice_angle_1 = 0.f;
    float lateral_lattice_angle_2 = 0.f;
    float infill_lock_depth       = 0;
    float skin_infill_depth       = 0;
    bool symmetric_infill_y_axis  = false;

    // Params for Lateral honeycomb
    float infill_overhang_angle = 60.f;

    // For Gyroid: when true, use the parameterized "optimized" wave.
    bool gyroid_optimized = false;

    // Orca: corner smoothing factor in the range [0, 1].
    double      smooth_factor { 0. };

    CenterOfSurfacePattern center_of_surface_pattern{CenterOfSurfacePattern::Each_Surface};
    bool                   separated_infills{false};

    // Orca: forced print order of surface fill loops/fragments for center-based patterns.
    SurfaceFillOrder fill_order = SurfaceFillOrder::Default;

	bool operator<(const SurfaceFillParams &rhs) const {
#define RETURN_COMPARE_NON_EQUAL(KEY) if (this->KEY < rhs.KEY) return true; if (this->KEY > rhs.KEY) return false;
#define RETURN_COMPARE_NON_EQUAL_TYPED(TYPE, KEY) if (TYPE(this->KEY) < TYPE(rhs.KEY)) return true; if (TYPE(this->KEY) > TYPE(rhs.KEY)) return false;

		// Sort first by decreasing bridging angle, so that the bridges are processed with priority when trimming one layer by the other.
		if (this->bridge_angle > rhs.bridge_angle) return true;
		if (this->bridge_angle < rhs.bridge_angle) return false;

		RETURN_COMPARE_NON_EQUAL(extruder);
		RETURN_COMPARE_NON_EQUAL_TYPED(unsigned, pattern);
        if (script_invocation < rhs.script_invocation) return true;
        if (rhs.script_invocation < script_invocation) return false;
		RETURN_COMPARE_NON_EQUAL(spacing);
		RETURN_COMPARE_NON_EQUAL(overlap);
		RETURN_COMPARE_NON_EQUAL(angle);
		RETURN_COMPARE_NON_EQUAL(fixed_angle);
		RETURN_COMPARE_NON_EQUAL(density);
		RETURN_COMPARE_NON_EQUAL(multiline);
//		RETURN_COMPARE_NON_EQUAL_TYPED(unsigned, dont_adjust);
		RETURN_COMPARE_NON_EQUAL(anchor_length);
		RETURN_COMPARE_NON_EQUAL(anchor_length_max);
		RETURN_COMPARE_NON_EQUAL(flow.width());
		RETURN_COMPARE_NON_EQUAL(flow.height());
		RETURN_COMPARE_NON_EQUAL(flow.nozzle_diameter());
		RETURN_COMPARE_NON_EQUAL_TYPED(unsigned, bridge);
		RETURN_COMPARE_NON_EQUAL_TYPED(unsigned, extrusion_role);
		RETURN_COMPARE_NON_EQUAL(role_speed);
        RETURN_COMPARE_NON_EQUAL(lateral_lattice_angle_1);
		RETURN_COMPARE_NON_EQUAL(lateral_lattice_angle_2);
		RETURN_COMPARE_NON_EQUAL(symmetric_infill_y_axis);
		RETURN_COMPARE_NON_EQUAL(infill_lock_depth);
		RETURN_COMPARE_NON_EQUAL(skin_infill_depth);
        RETURN_COMPARE_NON_EQUAL(infill_overhang_angle);
		RETURN_COMPARE_NON_EQUAL(gyroid_optimized);
        RETURN_COMPARE_NON_EQUAL(smooth_factor);
        RETURN_COMPARE_NON_EQUAL(center_of_surface_pattern);
        RETURN_COMPARE_NON_EQUAL(separated_infills);
		RETURN_COMPARE_NON_EQUAL_TYPED(unsigned, fill_order);

		return false;
	}

	bool operator==(const SurfaceFillParams &rhs) const {
		return  this->extruder 			      == rhs.extruder                &&
				this->pattern 			      == rhs.pattern                 &&
                this->script_invocation == rhs.script_invocation &&
				this->spacing 			      == rhs.spacing                 &&
				this->overlap 			      == rhs.overlap                 &&
				this->angle   			      == rhs.angle                   &&
				this->fixed_angle             == rhs.fixed_angle             &&
				this->bridge                  == rhs.bridge                  &&
				this->bridge_angle            == rhs.bridge_angle            &&
				this->density                 == rhs.density                 &&
				this->multiline               == rhs.multiline               &&
//				this->dont_adjust             == rhs.dont_adjust             &&
				this->anchor_length           == rhs.anchor_length           &&
				this->anchor_length_max       == rhs.anchor_length_max       &&
				this->flow                    == rhs.flow                    &&
				this->extrusion_role          == rhs.extrusion_role          &&
				this->role_speed              == rhs.role_speed              &&
                this->lateral_lattice_angle_1 == rhs.lateral_lattice_angle_1 &&
				this->lateral_lattice_angle_2 == rhs.lateral_lattice_angle_2 &&
				this->infill_lock_depth       == rhs.infill_lock_depth       &&
				this->skin_infill_depth       == rhs.skin_infill_depth       &&
                this->infill_overhang_angle   == rhs.infill_overhang_angle   &&
                this->center_of_surface_pattern == rhs.center_of_surface_pattern &&
                this->separated_infills       == rhs.separated_infills &&
                this->gyroid_optimized        == rhs.gyroid_optimized        &&
                this->smooth_factor           == rhs.smooth_factor           &&
                this->fill_order              == rhs.fill_order;
	}
};


#undef RETURN_COMPARE_NON_EQUAL
#undef RETURN_COMPARE_NON_EQUAL_TYPED

// Effective native substitution is a single operation: never leave a stale
// script fingerprint on helper-generated or narrow-concentric solid fills.
inline void set_native_effective_fill_pattern(SurfaceFillParams &params, InfillPattern native_pattern)
{
    if (native_pattern == ipScriptedPlanePath)
        throw InvalidArgument("Expected a native effective infill pattern");
    params.pattern = native_pattern;
    params.script_invocation = {};
}

} // namespace Slic3r
