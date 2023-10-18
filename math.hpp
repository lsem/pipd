#pragma

#include "types.hpp"
#include "v2.hpp"

namespace math {

inline Point closest_point_to_line(Point a, Point b, Point c) {
    v2 v{a, b};
    v2 u{a, c};
    v2 proj_v_u = projection_v_on_u(u, v);
    v2 n = normal(a);
    int sign = proj_v_u * v > 0 ? 1 : -1;
    v2 norm_proj_v_u = normalized(proj_v_u);

    double ratio = (len(proj_v_u) / len(v)) * sign;
    auto clamped_ratio = std::clamp(ratio, 0.0, 1.0);
    auto R = a + norm_proj_v_u * clamped_ratio * len(v);
    return R;
}

} // namespace math
