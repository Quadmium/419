#ifndef HITTABLE_H_
#define HITTABLE_H_
#include "ray.h"
#include "aabb.h"

struct HitResult {
  bool hit = false;
  vec3 point;
  vec3 normal;
  double t;
  vec3 albedo;
};

// Interface for being able to be hit by a ray
class Hittable {
  public:
    // Does ray r hit between t_min and t_max
    virtual HitResult hit(const Ray &r, double t_min, double t_max) = 0;
    // Bounding box
    virtual AABB aabb() = 0;
};

#endif // HITTABLE_H_