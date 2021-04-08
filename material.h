#ifndef MATERIAL_H_
#define MATERIAL_H_
#include "hittable.h"

struct ScatterResult {
  bool did = false;
  vec3 attenuation;
  Ray scattered;
};

class Material {
  public:
    virtual ScatterResult scatter(const Ray &ray, const HitResult &hit) const = 0;
    virtual vec3 emitted(const vec3& point) const {
      return {0, 0, 0};
    }
};

class Light : public Material {
  public:
    Light(const vec3 &albedo) : albedo(albedo) {}

    virtual ScatterResult scatter(const Ray &ray, const HitResult &hit) const override {
      return {};
    }

    virtual vec3 emitted(const vec3 &point) const {
      return albedo;
    }

    vec3 albedo;
};

class Lambertian : public Material {
  public:
    Lambertian(const vec3 &albedo) : albedo(albedo) {}

    virtual ScatterResult scatter(const Ray &ray, const HitResult &hit) const override {
      vec3 scatter_dir = hit.normal + random_unit_vector();

      if (scatter_dir.near_zero()) {
        scatter_dir = hit.normal;
      }

      return {true, albedo, Ray(hit.point, unit_vector(scatter_dir))};
    }

    vec3 albedo;
};

class Metal : public Material {
  public:
    Metal(const vec3 &albedo) : albedo(albedo) {}

    virtual ScatterResult scatter(const Ray &ray, const HitResult &hit) const override {
      vec3 reflected = reflect(unit_vector(ray.direction), hit.normal);
      Ray scattered = Ray(hit.point, reflected);
      return {dot(scattered.direction, hit.normal) > 0, albedo, scattered};
    }
    
    vec3 albedo;
};

class Dielectric : public Material {
  public:
    Dielectric(double ir) : ir(ir) {}

    virtual ScatterResult scatter(const Ray &ray, const HitResult &hit) const override {
      double refract_ratio = hit.front ? (1.0 / ir) : ir;

      vec3 unit_dir = unit_vector(ray.direction);
      double c = fmin(dot(-unit_dir, hit.normal), 1.0);
      double s = sqrt(1.0 - c * c);

      bool no_refract = refract_ratio * s > 1.0;
      vec3 dir;

      if (no_refract || reflectance(c, refract_ratio) > random_double()) {
        dir = reflect(unit_dir, hit.normal);
      } else {
        dir = refract(unit_dir, hit.normal, refract_ratio);
      }

      return {true, {1.0, 1.0, 1.0}, Ray(hit.point, dir)};
    }

    double ir;

    private:
      static double reflectance(double c, double refract_ratio) {
        double r0 = (1 - refract_ratio) / (1 + refract_ratio);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - c), 5);
      }
};

class Checkers : public Material {
  public:
    Checkers(const vec3 &albedo1, const vec3 &albedo2, double scale=2.0) : albedo1(albedo1), albedo2(albedo2), scale(scale) {}

    virtual ScatterResult scatter(const Ray &ray, const HitResult &hit) const override {
      vec3 scatter_dir = hit.normal + random_unit_vector();

      if (scatter_dir.near_zero()) {
        scatter_dir = hit.normal;
      }

      // Shift to avoid double lines at origin
      // Scale to increase pattern frequency
      int x = 10000 + hit.point.x() * scale;
      int z = 10000 + hit.point.z() * scale;

      vec3 final_albedo = albedo1;

      if (x % 2 == z % 2) {
        final_albedo = albedo2;
      }

      return {true, final_albedo, Ray(hit.point, scatter_dir)};
    }

    vec3 albedo1;
    vec3 albedo2;
    double scale;
};
#endif