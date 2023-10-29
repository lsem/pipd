#pragma

#include "types.hpp"
#include "v2.hpp"

namespace math {

inline double points_distance(Point a, Point b) { return len(v2{a, b}); }

inline Point closest_point_to_line(Point a, Point b, Point c) {
    v2 v{a, b};
    v2 u{a, c};
    v2 proj_v_u = projection_v_on_u(u, v);
    int sign = proj_v_u * v > 0 ? 1 : -1;
    v2 norm_proj_v_u = normalized(proj_v_u);
    double ratio = (len(proj_v_u) / len(v)) * sign;
    auto clamped_ratio = std::clamp(ratio, 0.0, 1.0);
    auto R = a + norm_proj_v_u * clamped_ratio * len(v);
    return R;
}

int wrap_index(size_t index, size_t n) { return ((index % n) + n) % n; }

inline bool rect_point_hit_test(std::array<Point, 4> rect, Point p) {
    for (size_t i = 1; i < 5; ++i) {
        Point p1 = rect[wrap_index(i - 1, 4)];
        Point p2 = rect[wrap_index(i, 4)];
        v2 v{p1, p2};
        v2 u{p1, p};
        v2 pv = normal(v);
        double d = dot(pv, u);
        int sign = d > 0 ? 1 : -1;
        if (sign < 0) {
            return false;
        }
    }

    return true;
}

inline double angle_between_vectors(::v2 v1, ::v2 v2) {
    double theta = std::atan2(cross2d(v1, v2), dot(v1, v2));
    if (theta < 0) {
        theta += 2 * M_PI;
    }
    return theta;
}

} // namespace math
