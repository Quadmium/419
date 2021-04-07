#ifndef HITTABLE_H_
#define HITTABLE_H_
#include "ray.h"
#include "aabb.h"
#include <memory>

class Material;

struct HitResult {
  bool hit = false;
  vec3 point;
  vec3 normal;
  double t;
  bool front;
  std::shared_ptr<Material> material;

  inline void set_normal(const Ray &r, const vec3 &outward_normal) {
    front = dot(r.direction, outward_normal) < 0;
    normal = front ? outward_normal :-outward_normal;
  }
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