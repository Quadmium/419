#ifndef RECTANGLE_H_
#define RECTANGLE_H_
#include "hittable.h"
#include "material.h"

class Rectangle : public Hittable {
  public:
    Rectangle(double x0, double x1, double y0, double y1, double z, std::shared_ptr<Material> material) : x0(x0), x1(x1), y0(y0), y1(y1), z(z), material(material) {}

    virtual HitResult hit(const Ray &r, double t_min, double t_max) {
      HitResult res = {};
      auto t = (z-r.origin.z()) / r.direction.z();
      if (t < t_min || t > t_max)
          return res;
      auto x = r.origin.x() + t*r.direction.x();
      auto y = r.origin.y() + t*r.direction.y();
      if (x < x0 || x > x1 || y < y0 || y > y1)
          return res;
      res.hit = true;
      res.t = t;
      auto outward_normal = vec3(0, 0, 1);
      res.set_normal(r, outward_normal);
      res.material = material;
      res.point = r.at(t);
      return res;
    }
    
    virtual AABB aabb() {
      return {
        {x0, y0, z - 0.001},
        {x1, y1, z + 0.001}
      };
    }

    double x0, x1, y0, y1, z;
    std::shared_ptr<Material> material;
};
#endif