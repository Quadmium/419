#include <iostream>
#include <cmath>
#include <vector>
#include <memory>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <unordered_map>

#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "triangle.h"
#include "bvh.h"
#include "material.h"
#include "rectangle.h"

// Assigns a vec3 to char*, used for assigning float pixels to discrete images
// img - target array
// color - color to assign in [0,1] range
void img_assign(char *img, const vec3 &color) {
  img[0] = 255.999 * std::sqrt(color.e[0]);
  img[1] = 255.999 * std::sqrt(color.e[1]);
  img[2] = 255.999 * std::sqrt(color.e[2]);
}

// Checks if a ray hits a plane
// anchor - anchor of plane
// normal - normal of plane
// r - ray to test
// returns t of hit
double hit_plane(const vec3 &anchor, const vec3 &normal, const Ray &r) {
  double denominator = dot(r.direction, normal);
  if (abs(denominator) < 0.00001) {
    return -1;
  }
  double t = dot(anchor - r.origin, normal) / denominator;
  return t;
}

// Uses a BVH, returns color
// r - ray to shoot
// bvh_root - bvh root node
vec3 shoot_ray(const Ray &r, BVHNode &bvh_root, int depth) {
  if (depth == 0) {
    return {0, 0, 0};
  }

  HitResult hit = bvh_root.hit(r, 0.001, 1000);

  if (!hit.hit) {
    double t = std::max(std::min(r.direction.y(), 1.0), 0.0);
    //printf("%f\n", t);
    return (1-t) * vec3{0.33, 0.61, 0.72} + (t) * vec3{0.9, 0.9, 0.72};
  }

  vec3 emitted = hit.material->emitted(hit.point);

  ScatterResult s = hit.material->scatter(r, hit);
  if (!s.did) {
    return emitted;
  }

  return emitted + s.attenuation * shoot_ray(s.scattered, bvh_root, depth-1);
}

// Generates a random number between [min, max]
int rand_int(int min, int max) {
  return rand() % (max - min + 1) + min;
}

// Holds row and column in [0, 1]
struct Sample {
  double r;
  double c;
};

int main(int argc, char **argv) {
  // Output params
  const size_t width = 1000;
  const size_t height = 1000;
  const size_t channels = 3;
  char png[height][width][channels] = {};

  // Switch this if needed
  bool is_ortho = false;
  
  // For mesh
  vec3 camera_pos = {0, 1, 0};
  vec3 camera_forward = unit_vector(vec3(0, 0, -10) - camera_pos);

  // Slightly different viewpoint for the ortho images
  if (is_ortho) {
    camera_pos = {4 * std::sin(0 / 20.0), 2, 4 * std::cos(0 / 20.0)};
    camera_forward = unit_vector(vec3(0, 1, -2) - camera_pos);
  }

  // Calculate camera-local axis
  vec3 camera_right = cross(camera_forward, {0, 1, 0});
  vec3 camera_up = cross(camera_right, camera_forward);

  // Calculate viewport vectors
  double aspect_ratio = static_cast<double>(width) / height;
  double focal = 1;

  double viewport_height = 1;
  if (is_ortho) {
    viewport_height *= 3.5;
  }

  // These are used to get world coords of pixels in viewport
  double viewport_width = viewport_height * aspect_ratio;
  vec3 viewport_right = viewport_width * camera_right;
  vec3 viewport_down = -viewport_height * camera_up;
  vec3 viewport_top_left = camera_pos - viewport_right / 2 - viewport_down / 2 + focal * camera_forward;

  // Number of multi jitter samples = n^2
  size_t n = 10;
  double n_d = n;
  Sample samples[n][n] = {};

  std::vector<std::unique_ptr<Hittable>> world;

  auto material_ground = std::make_shared<Checkers>(color(1, 0, 0), color(1, 1, 0));
  auto material_light = std::make_shared<Light>(vec3(1, 1, 1));
  auto material_glass   = std::make_shared<Dielectric>(1.5);
  auto material_metal  = std::make_shared<Metal>(color(0.5, 0.5, 0.5));
  auto material_red = std::make_shared<Lambertian>(color(1, 0, 0));
  auto material_green = std::make_shared<Lambertian>(color(0, 1, 0));
  auto material_blue = std::make_shared<Lambertian>(color(0, 0, 1));

  world.push_back(std::make_unique<Sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
  world.push_back(std::make_unique<Sphere>(point3( 0,    0.5, -2.5),   0.5, material_glass));
  world.push_back(std::make_unique<Sphere>(point3( 0,    0.5, -2.5),   -0.45, material_glass));
  world.push_back(std::make_unique<Sphere>(point3( 1,    0, -3.5),   0.5, material_metal));
  world.push_back(std::make_unique<Sphere>(point3( -1,    0, -3.5),   0.5, material_blue));

  // Convert to list of pointers for bvh
  std::vector<Hittable*> world_ptrs;
  for (size_t i = 0; i < world.size(); ++i) {
    world_ptrs.push_back(world[i].get());
  }
  
  BVHNode bvh_root(world_ptrs);

  for (size_t r = 0; r < height; ++r) {
    for (size_t c = 0; c < width; ++c) {
      // Generate grid of samples diagonally by row
      for (size_t rr = 0; rr < n; ++rr) {
        for (size_t cc = 0; cc < n; ++cc) {
          samples[rr][cc].r = rr / n_d + (cc % n) / n_d / n_d + 0.5 / n_d / n_d;
          samples[rr][cc].c = cc / n_d + (rr % n) / n_d / n_d + 0.5 / n_d / n_d;
        }
      }

      // Shuffle samples[r][...].r
      for (size_t r = 0; r < n; ++r) {
        for (size_t i = n - 1; i >= 1; --i) {
          int rand = rand_int(0, i);
          std::swap(samples[r][i].r, samples[r][rand].r);
        }
      }

      // Shuffle samples[...][c].c
      for (size_t c = 0; c < n; ++c) {
        for (size_t i = n - 1; i >= 1; --i) {
          int rand = rand_int(0, i);
          std::swap(samples[i][c].c, samples[rand][c].c);
        }
      }

      // Sum results of all samples
      vec3 color_sum = {};

      for (size_t r_s = 0; r_s < n; ++r_s) {
        for (size_t c_s = 0; c_s < n; ++c_s) {
          // Position within viewport plus jitter
          double row_ratio = (static_cast<double>(r) + samples[r_s][c_s].r) / height;
          double col_ratio = (static_cast<double>(c) + samples[r_s][c_s].c) / width;
          // For perspective shoot from camera towards viewport
          Ray ray = {camera_pos, viewport_top_left + viewport_down * row_ratio + viewport_right * col_ratio - camera_pos};
          if (is_ortho) {
            // For orthographic shoot forwards from viewport
            ray = {viewport_top_left + viewport_down * row_ratio + viewport_right * col_ratio, camera_forward};
          }
          // Change to use slower approach
          //color_sum += shoot_ray_slow(ray, world);
          color_sum += shoot_ray(ray, bvh_root, 30);
        }
      }

      // Assign final color
      img_assign(png[r][c], color_sum / (n * n));
    }
  }
  
  // Write image
  stbi_write_png("out/transparent.png", width, height, channels, png, width * channels);
  return 0;
}
