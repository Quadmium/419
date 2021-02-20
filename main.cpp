#include <iostream>
#include <cmath>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vec3.h"
#include "ray.h"

void img_assign(char *img, const vec3 &color) {
  img[0] = 255.999 * color.e[0];
  img[1] = 255.999 * color.e[1];
  img[2] = 255.999 * color.e[2];
}

double hit_sphere(const point3& center, double radius, const Ray& r) {
    vec3 oc = r.origin - center;
    auto a = r.direction.length_squared();
    auto half_b = dot(oc, r.direction);
    auto c = oc.length_squared() - radius*radius;
    auto discriminant = half_b*half_b - a*c;

    if (discriminant < 0) {
        return -1.0;
    } else {
        return (-half_b - sqrt(discriminant) ) / a;
    }
}

bool hit_sphere(const point3& center, double radius, const Ray& r, double *t0, double *t1) {
  vec3 d = r.direction;
  vec3 d_unit = unit_vector(d);
  vec3 f = r.origin - center;
  double a = d.length_squared();
  double b = 2 * dot(f, d);
  double c = f.length_squared() - radius * radius;

  double b2_minus_4ac = 4 * a * (radius * radius - (f - dot(f, d_unit) * d_unit).length_squared());

  if (b2_minus_4ac < 0) {
    return false;
  }

  double q = -0.5 * (b + (b >= 0 ? 1 : -1) * std::sqrt(b2_minus_4ac));

  *t0 = c / q;
  *t1 = q / a;

  // Order them
  if (*t1 < *t0) {
    std::swap(*t0, *t1);
  }

  // Try putting t1 first if t0 is negative
  if (*t0 < 0) {
    std::swap(*t0, *t1);
  }

  // If still negative, both are negative, put it back and return
  if (*t0 < 0) {
    std::swap(*t0, *t1);
    return false;
  }

  // Otherwise, t0 is positive
  return true;
}

double hit_plane(const vec3 &anchor, const vec3 &normal, const Ray &r) {
  double denominator = dot(r.direction, normal);
  if (denominator == 0.0) {
    denominator = 0.0000001;
  }
  return dot(anchor - r.origin, normal) / denominator;
}

vec3 shoot_ray(const Ray &r) {
  vec3 anchor = {0, -1, 0};
  vec3 normal = {0, 1, 0};

  double t0, t1;
  if (hit_sphere({0, 0, -2}, 0.5, r, &t0, &t1)) {
    vec3 N = unit_vector(r.at(t0) - vec3(0,0,-2));
    return 0.5*color(N.x()+1, N.y()+1, N.z()+1);
  }

  if (hit_plane(anchor, normal, r) > 0) {
    return r.at(hit_plane(anchor, normal, r));
  }

  return {0, 0, 0};
}

int main(int argc, char **argv) {
  const size_t width = 400;
  const size_t height = 225;
  const size_t channels = 3;
  char png[height][width][channels] = {};

  double aspect_ratio = static_cast<double>(width) / height;
  double focal = 1.0;

  double viewport_height = 1;
  double viewport_width = viewport_height * aspect_ratio;
  vec3 viewport_right = {viewport_width, 0, 0};
  vec3 viewport_down = {0, -viewport_height, 0};

  vec3 origin = {};
  vec3 viewport_top_left = origin - viewport_right / 2 - viewport_down / 2 - vec3(0, 0, focal);

  for (size_t r = 0; r < height; ++r) {
    for (size_t c = 0; c < width; ++c) {
      double row_ratio = (static_cast<double>(r) + 0.5) / height;
      double col_ratio = (static_cast<double>(c) + 0.5) / width;
      Ray ray = {origin, viewport_top_left + viewport_down * row_ratio + viewport_right * col_ratio};
      img_assign(png[r][c], shoot_ray(ray));
      //png[r][c][0] = (char)(255*(ray.direction.e[1] + viewport_height / 2) / viewport_height);
    }
  }
  
  stbi_write_png("out/test.png", width, height, channels, png, width * channels);
  return 0;
}