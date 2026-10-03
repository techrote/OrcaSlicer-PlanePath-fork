#include <catch2/catch_all.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

#include "libslic3r/Fill/FillPlanePath.hpp"
#include "libslic3r/PrintConfig.hpp"
#include "libslic3r/Surface.hpp"

using namespace Slic3r;

namespace {

constexpr double output_scale = 1'000'000.;

class TestableHilbertCurve : public FillHilbertCurve
{
public:
    Points generate_points(double resolution, double smooth_factor = 0., coord_t max_coordinate = 7)
    {
        return generate_points(0, 0, max_coordinate, max_coordinate, resolution, smooth_factor);
    }

    Points generate_points(coord_t min_x, coord_t min_y, coord_t max_x, coord_t max_y,
                           double resolution, double smooth_factor = 0.)
    {
        InfillPolylineOutput output(output_scale);
        FillParams params;
        params.smooth_factor = smooth_factor;
        FillHilbertCurve::generate(min_x, min_y, max_x, max_y, resolution, params, output);
        return std::move(output.result());
    }

    bool centered_value() const { return centered(); }
};

class TestableOctagramSpiral : public FillOctagramSpiral
{
public:
    Points generate_points(double resolution, double smooth_factor = 0., coord_t max_coordinate = 7)
    {
        InfillPolylineOutput output(output_scale);
        FillParams params;
        params.smooth_factor = smooth_factor;
        FillOctagramSpiral::generate(-max_coordinate, -max_coordinate, max_coordinate, max_coordinate, resolution, params, output);
        return std::move(output.result());
    }

    bool centered_value() const { return centered(); }
};

class TestableArchimedeanChords : public FillArchimedeanChords
{
public:
    bool centered_value() const { return centered(); }
};

class OutputModeProbe : public FillPlanePath
{
public:
    Fill *clone() const override { return new OutputModeProbe(*this); }

    bool run_for(ExtrusionRole role)
    {
        m_saw_clipping_output = false;
        spacing = 1.;
        overlap = 0.5;
        angle = float(-M_PI / 2.);
        fixed_angle = true;

        Points square{
            Point::new_scale(-5., -5.),
            Point::new_scale( 5., -5.),
            Point::new_scale( 5.,  5.),
            Point::new_scale(-5.,  5.)
        };
        ExPolygon expolygon(square);
        set_bounding_box(expolygon.contour.bounding_box());

        FillParams params;
        params.density = 1.f;
        params.extrusion_role = role;
        Surface surface(stInternal, expolygon);
        (void)fill_surface(&surface, params);
        return m_saw_clipping_output;
    }

protected:
    bool centered() const override { return false; }

    void generate(coord_t min_x, coord_t min_y, coord_t max_x, coord_t max_y,
                  const double, InfillPolylineOutput &output) override
    {
        m_saw_clipping_output = output.clips();
        output.add_point({double(min_x), double(min_y)});
        output.add_point({double(max_x), double(max_y)});
    }

private:
    bool m_saw_clipping_output { false };
};

Points scaled_points(const std::vector<std::pair<coord_t, coord_t>> &grid, coord_t dx = 0, coord_t dy = 0)
{
    Points out;
    out.reserve(grid.size());
    for (const auto &[x, y] : grid)
        out.emplace_back(coord_t((x + dx) * output_scale), coord_t((y + dy) * output_scale));
    return out;
}

// Cosine of the sharpest turn of a path, 1 meaning it has no turn at all.
double sharpest_turn_cosine(const Points &points)
{
    double sharpest = 1.;
    for (size_t i = 1; i + 1 < points.size(); ++i) {
        const Vec2d incoming = (points[i] - points[i - 1]).cast<double>().normalized();
        const Vec2d outgoing = (points[i + 1] - points[i]).cast<double>().normalized();
        sharpest = std::min(sharpest, incoming.dot(outgoing));
    }
    return sharpest;
}

double path_length(const Points &points)
{
    double length = 0.;
    for (size_t i = 1; i < points.size(); ++i)
        length += (points[i] - points[i - 1]).cast<double>().norm();
    return length;
}

double discrete_curvature_at(const Points &points, const Point &point)
{
    const auto point_it = std::find(points.begin(), points.end(), point);
    REQUIRE(point_it != points.end());
    const size_t point_idx = size_t(std::distance(points.begin(), point_it));
    REQUIRE(point_idx > 0);
    REQUIRE(point_idx + 1 < points.size());

    const Vec2d incoming = (points[point_idx] - points[point_idx - 1]).cast<double>() / output_scale;
    const Vec2d outgoing = (points[point_idx + 1] - points[point_idx]).cast<double>() / output_scale;
    const Vec2d chord     = incoming + outgoing;
    const double cross   = std::abs(incoming.x() * outgoing.y() - incoming.y() * outgoing.x());
    return 2. * cross / (incoming.norm() * outgoing.norm() * chord.norm());
}

} // namespace


TEST_CASE("Native Hilbert raw path matches golden power-of-two fixtures", "[FillPlanePath][PlanePathConformance]")
{
    const std::vector<std::pair<coord_t, coord_t>> order_1{
        {0, 0}, {0, 1}, {1, 1}, {1, 0}
    };
    const std::vector<std::pair<coord_t, coord_t>> order_2{
        {0, 0}, {0, 1}, {1, 1}, {1, 0},
        {2, 0}, {3, 0}, {3, 1}, {2, 1},
        {2, 2}, {3, 2}, {3, 3}, {2, 3},
        {1, 3}, {1, 2}, {0, 2}, {0, 3}
    };

    struct Fixture {
        coord_t min_x;
        coord_t min_y;
        coord_t max_x;
        coord_t max_y;
        double resolution;
        const std::vector<std::pair<coord_t, coord_t>> *golden;
    };
    const std::array<Fixture, 5> fixtures{{
        { 0,  0, 1, 1, 0.5,   &order_1},
        { 0,  0, 2, 1, 0.125, &order_2},
        { 0,  0, 3, 3, 0.001, &order_2},
        {-2,  3, 1, 4, 0.25,  &order_2},
        {-2, -3, 1, 0, 1.0,   &order_2}
    }};

    for (const Fixture &fixture : fixtures) {
        CAPTURE(fixture.min_x, fixture.min_y, fixture.max_x, fixture.max_y, fixture.resolution);
        const Points actual = TestableHilbertCurve().generate_points(
            fixture.min_x, fixture.min_y, fixture.max_x, fixture.max_y, fixture.resolution);
        const Points expected = scaled_points(*fixture.golden, fixture.min_x, fixture.min_y);
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Native Hilbert raw coordinates are independent of smoothing resolution", "[FillPlanePath][PlanePathConformance]")
{
    const Points coarse = TestableHilbertCurve().generate_points(1.0, 0., 7);
    const Points fine   = TestableHilbertCurve().generate_points(0.0001, 0., 7);
    REQUIRE(coarse == fine);
}

TEST_CASE("Native Hilbert raw path is a unit-step Hamiltonian traversal", "[FillPlanePath][PlanePathConformance]")
{
    struct Fixture {
        coord_t max_coordinate;
        size_t side;
        coord_t end_x;
        coord_t end_y;
    };
    const std::array<Fixture, 3> fixtures{{{1, 2, 1, 0}, {3, 4, 0, 3}, {7, 8, 7, 0}}};

    for (const Fixture &fixture : fixtures) {
        CAPTURE(fixture.max_coordinate, fixture.side);
        const Points points = TestableHilbertCurve().generate_points(0.0125, 0., fixture.max_coordinate);
        REQUIRE(points.size() == fixture.side * fixture.side);
        REQUIRE(points.front() == Point(0, 0));
        REQUIRE(points.back() == Point(coord_t(fixture.end_x * output_scale), coord_t(fixture.end_y * output_scale)));

        std::set<std::pair<coord_t, coord_t>> visited;
        for (size_t i = 0; i < points.size(); ++i) {
            const Point &point = points[i];
            REQUIRE(point.x() >= 0);
            REQUIRE(point.y() >= 0);
            REQUIRE(point.x() <= coord_t((fixture.side - 1) * output_scale));
            REQUIRE(point.y() <= coord_t((fixture.side - 1) * output_scale));
            REQUIRE(point.x() % coord_t(output_scale) == 0);
            REQUIRE(point.y() % coord_t(output_scale) == 0);
            visited.emplace(point.x(), point.y());
            if (i > 0) {
                const Point delta = points[i] - points[i - 1];
                REQUIRE(std::abs(delta.x()) + std::abs(delta.y()) == coord_t(output_scale));
            }
        }
        REQUIRE(visited.size() == points.size());
    }
}

TEST_CASE("Native PlanePath capability decisions are frozen before trait refactoring", "[FillPlanePath][PlanePathConformance]")
{
    STATIC_REQUIRE(std::is_trivially_copyable_v<FillParams>);

    TestableHilbertCurve hilbert;
    TestableArchimedeanChords archimedean;
    TestableOctagramSpiral octagram;

    CHECK_FALSE(hilbert.centered_value());
    CHECK(archimedean.centered_value());
    CHECK(octagram.centered_value());

    struct PatternDecision {
        InfillPattern pattern;
        bool smoothable;
    };
    const std::array<PatternDecision, 3> patterns{{
        {ipHilbertCurve, true},
        {ipArchimedeanChords, false},
        {ipOctagramSpiral, true}
    }};

    for (const PatternDecision &decision : patterns) {
        CAPTURE(decision.pattern);
        CHECK(is_separable_infill_pattern(decision.pattern));
        CHECK(is_smoothable_infill_pattern(decision.pattern) == decision.smoothable);
        CHECK_FALSE(Fill::use_bridge_flow(decision.pattern));

        std::unique_ptr<Fill> filler(Fill::new_from_type(decision.pattern));
        REQUIRE(filler != nullptr);
        CHECK_FALSE(filler->is_self_crossing());
        CHECK_FALSE(filler->no_sort());
        CHECK_FALSE(filler->use_bridge_flow());
    }
}

TEST_CASE("Sparse PlanePath selects the clipping output while solid PlanePath does not", "[FillPlanePath][PlanePathConformance]")
{
    OutputModeProbe sparse;
    OutputModeProbe solid;

    CHECK(sparse.run_for(erInternalInfill));
    CHECK_FALSE(solid.run_for(erSolidInfill));
}

TEST_CASE("Hilbert curve exposes a smoothing factor", "[FillPlanePath]")
{
    const ConfigOptionDef *factor_def = print_config_def.get("sparse_infill_smooth_factor");
    REQUIRE(factor_def != nullptr);
    REQUIRE(factor_def->type == coPercent);
    REQUIRE_THAT(factor_def->min, Catch::Matchers::WithinAbs(0., 1e-12));
    REQUIRE_THAT(factor_def->max, Catch::Matchers::WithinAbs(100., 1e-12));
    REQUIRE_THAT(factor_def->get_default_value<ConfigOptionPercent>()->value,
                 Catch::Matchers::WithinAbs(0., 1e-12));
}

TEST_CASE("Hilbert curve smoothing rounds right angle turns", "[FillPlanePath]")
{
    const Points sharp  = TestableHilbertCurve().generate_points(0.005);
    const Points smooth = TestableHilbertCurve().generate_points(0.005, 1.);

    REQUIRE(smooth.front() == sharp.front());
    REQUIRE(smooth.back() == sharp.back());
    REQUIRE(smooth.size() > sharp.size());

    bool has_turn = false;
    for (size_t i = 1; i < smooth.size(); ++i) {
        const Vec2d segment = (smooth[i] - smooth[i - 1]).cast<double>();
        REQUIRE(segment.squaredNorm() > 0.);
    }
    for (size_t i = 1; i + 1 < smooth.size(); ++i) {
        const Vec2d incoming = (smooth[i] - smooth[i - 1]).cast<double>();
        const Vec2d outgoing = (smooth[i + 1] - smooth[i]).cast<double>();
        const double cross   = incoming.x() * outgoing.y() - incoming.y() * outgoing.x();
        const double cosine  = incoming.dot(outgoing) / (incoming.norm() * outgoing.norm());
        has_turn |= std::abs(cross) > 0.;
        REQUIRE(cosine > 0.);
    }
    REQUIRE(has_turn);

    const coord_t upper_bound = coord_t(7 * output_scale);
    for (const Point &point : smooth) {
        REQUIRE(point.x() >= 0);
        REQUIRE(point.y() >= 0);
        REQUIRE(point.x() <= upper_bound);
        REQUIRE(point.y() <= upper_bound);
    }
}

TEST_CASE("Smoothed Hilbert curve honors path resolution", "[FillPlanePath]")
{
    const Points coarse = TestableHilbertCurve().generate_points(0.1, 1.);
    const Points fine   = TestableHilbertCurve().generate_points(0.001, 1.);

    REQUIRE(fine.size() > coarse.size());
    REQUIRE(fine.front() == coarse.front());
    REQUIRE(fine.back() == coarse.back());
}

TEST_CASE("Smoothed Hilbert corners use a uniform subdivision depth", "[FillPlanePath]")
{
    const Points smooth = TestableHilbertCurve().generate_points(0.0035, 1., 1);
    const Point  curve_entry(0, coord_t(0.5 * output_scale));
    const Point  curve_exit(coord_t(0.5 * output_scale), coord_t(output_scale));

    const auto entry_it = std::find(smooth.begin(), smooth.end(), curve_entry);
    REQUIRE(entry_it != smooth.end());
    const auto exit_it = std::find(entry_it, smooth.end(), curve_exit);
    REQUIRE(exit_it != smooth.end());

    const size_t segment_count = size_t(std::distance(entry_it, exit_it));
    REQUIRE(segment_count > 1);
    REQUIRE((segment_count & (segment_count - 1)) == 0);

    double previous_length = (entry_it[1] - entry_it[0]).cast<double>().norm();
    REQUIRE(previous_length > 0.);
    double max_length_ratio = 1.;
    for (size_t segment = 1; segment < segment_count; ++segment) {
        const double current_length = (entry_it[segment + 1] - entry_it[segment]).cast<double>().norm();
        REQUIRE(current_length > 0.);
        max_length_ratio = std::max(max_length_ratio,
                                    std::max(current_length / previous_length, previous_length / current_length));
        previous_length = current_length;
    }
    REQUIRE(max_length_ratio < 1.5);
}

TEST_CASE("Hilbert smoothing joins straight segments with continuous curvature", "[FillPlanePath]")
{
    const Points coarse = TestableHilbertCurve().generate_points(0.005, 0.5, 1);
    const Points fine   = TestableHilbertCurve().generate_points(0.0001, 0.5, 1);
    const Point  first_curve_entry(0, coord_t(0.75 * output_scale));

    const double coarse_entry_curvature = discrete_curvature_at(coarse, first_curve_entry);
    const double fine_entry_curvature   = discrete_curvature_at(fine, first_curve_entry);
    REQUIRE(coarse_entry_curvature > 0.);
    REQUIRE(fine_entry_curvature < 0.25 * coarse_entry_curvature);
}

TEST_CASE("Octagram spiral smoothing rounds the turns of the spiral", "[FillPlanePath]")
{
    const Points sharp  = TestableOctagramSpiral().generate_points(0.005);
    const Points smooth = TestableOctagramSpiral().generate_points(0.005, 1.);

    REQUIRE(smooth.size() > sharp.size());
    REQUIRE(smooth.front() == sharp.front());
    REQUIRE(smooth.back() == sharp.back());
    // The spiral alternates between 90 and 135 degree turns; both are rounded into gentle ones.
    REQUIRE(sharpest_turn_cosine(sharp) < -0.7);
    REQUIRE(sharpest_turn_cosine(smooth) > 0.9);

    for (size_t i = 1; i < smooth.size(); ++i)
        REQUIRE((smooth[i] - smooth[i - 1]).cast<double>().squaredNorm() > 0.);
}

TEST_CASE("Octagram spiral smooth factor controls corner curvature", "[FillPlanePath]")
{
    const Points sharp          = TestableOctagramSpiral().generate_points(0.005);
    const Points half_smooth    = TestableOctagramSpiral().generate_points(0.005, 0.5);
    const Points full_smooth    = TestableOctagramSpiral().generate_points(0.005, 1.);
    const Points invalid_factor = TestableOctagramSpiral().generate_points(
        0.005, std::numeric_limits<double>::quiet_NaN());

    REQUIRE(path_length(full_smooth) < path_length(half_smooth));
    REQUIRE(path_length(half_smooth) < path_length(sharp));
    REQUIRE(invalid_factor == sharp);
}

TEST_CASE("Hilbert curve smooth factor controls corner curvature", "[FillPlanePath]")
{
    const Points sharp       = TestableHilbertCurve().generate_points(0.005);
    const Points half_smooth = TestableHilbertCurve().generate_points(0.005, 0.5);
    const Points full_smooth = TestableHilbertCurve().generate_points(0.005, 1.);
    const Points invalid_factor = TestableHilbertCurve().generate_points(
        0.005, std::numeric_limits<double>::quiet_NaN());

    REQUIRE(full_smooth.front() == half_smooth.front());
    REQUIRE(full_smooth.back() == half_smooth.back());
    REQUIRE(path_length(full_smooth) < path_length(half_smooth));
    REQUIRE(invalid_factor == sharp);

    for (size_t i = 1; i < full_smooth.size(); ++i)
        REQUIRE((full_smooth[i] - full_smooth[i - 1]).squaredNorm() > 0);
}
