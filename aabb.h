#ifndef AABB_H_
#define AABB_H_
#include "ray.h"

// A bounding box
class AABB {
  public:
    AABB() {}
    // min - smallest coordinate, max - biggest coordinate
    AABB(const vec3 &min, const vec3 &max) : min(min), max(max), center((min + max) / 2) {}

    // Test if ray r hits aabb within t_min to t_max
    // https://raytracing.github.io/books/RayTracingTheNextWeek.html#boundingvolumehierarchies/axis-alignedboundingboxes(aabbs)
    inline bool hit(const Ray& r, double t_min, double t_max) {
      for (int a = 0; a < 3; a++) {
          auto invD = 1.0f / r.direction[a];
          auto t0 = (min[a] - r.origin[a]) * invD;
          auto t1 = (max[a] - r.origin[a]) * invD;
          if (invD < 0.0f)
              std::swap(t0, t1);
          t_min = t0 > t_min ? t0 : t_min;
          t_max = t1 < t_max ? t1 : t_max;
          if (t_max <= t_min)
              return false;
      }
      return true;
    }

    // Return smallest AABB encapsulating this and other
    AABB merge(const AABB &other) {
      return {
        min_e(min, other.min),
        max_e(max, other.max)
      };
    }

    vec3 min;
    vec3 max;
    vec3 center;
};

#endif