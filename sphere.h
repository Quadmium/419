#ifndef SPHERE_H_
#define SPHERE_H_
#include "hittable.h"

class Sphere : public Hittable {
  public:
    // center of sphere, radius of sphere, material of sphere
    Sphere(vec3 center, double radius, std::shared_ptr<Material> material) : center(center), radius(radius), material(material) {}

    // Ray r hits sphere between t_min and t_max
    HitResult hit(const Ray& r, double t_min, double t_max) {
      HitResult res = {};
      
      // Adapted from lecture
      vec3 d = r.direction;
      vec3 d_unit = unit_vector(d);
      vec3 f = r.origin - center;
      double a = d.length_squared();
      double b = 2 * dot(f, d);
      double c = f.length_squared() - radius * radius;

      double b2_minus_4ac = 4 * a * (radius * radius - (f - dot(f, d_unit) * d_unit).length_squared());

      // Return early for invaid determinant
      if (b2_minus_4ac < 0) {
        return res;
      }

      double q = -0.5 * (b + (b >= 0 ? 1 : -1) * std::sqrt(b2_minus_4ac));

      // Calculate two solutions
      double t0 = c / q;
      double t1 = q / a;

      bool t0_valid = t0 >= t_min && t0 <= t_max;
      bool t1_valid = t1 >= t_min && t1 <= t_max;

      if (t0_valid && t1_valid) {
        res.t = std::min(t0, t1);
      } else if (t0_valid) {
        res.t = t0;
      } else if (t1_valid) {
        res.t = t1;
      } else {
        return res;
      }

      res.hit = true;
      res.point = r.at(res.t);
      res.set_normal(r, unit_vector(res.point - center));
      res.material = material;

      return res;
    }

    // Bounding box of sphere
    virtual AABB aabb() {
      return {
        center - vec3(radius, radius, radius),
        center + vec3(radius, radius, radius)
      };
    }
  
    vec3 center;
    double radius;
    std::shared_ptr<Material> material;
};

#endif