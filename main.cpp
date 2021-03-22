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

// Assigns a vec3 to char*, used for assigning float pixels to discrete images
// img - target array
// color - color to assign in [0,1] range
void img_assign(char *img, const vec3 &color) {
  img[0] = 255.999 * color.e[0];
  img[1] = 255.999 * color.e[1];
  img[2] = 255.999 * color.e[2];
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

// Uses traditional check every hittable approach
// r - ray to shoot
// world - list of all hittable objects
vec3 shoot_ray_slow(const Ray &r, std::vector<std::unique_ptr<Hittable>> &world) {
  HitResult final;
  final.t = 10000;
  for (std::unique_ptr<Hittable> &h : world) {
    HitResult res = h->hit(r, 0, 1000);
    if (res.hit && res.t < final.t) {
      final = res;
    }
  }

  if (!final.hit) {
    return {0, 0, 0};
  }

  vec3 light = {10, 10, 10};
  vec3 to_light = unit_vector(light - final.point);
  double diffuse = std::max(dot(to_light, final.normal), 0.0);

  return diffuse * final.albedo;
}

// Uses a BVH, returns color
// r - ray to shoot
// bvh_root - bvh root node
vec3 shoot_ray(const Ray &r, BVHNode &bvh_root) {
  HitResult final = bvh_root.hit(r, 0, 1000);

  // Additional logic to make plane
  double plane_time = hit_plane({0, -0.57, 0}, {0, 1, 0}, r);

  vec3 bg_color = vec3{135, 206, 235} / 255.0;
  vec3 plane_color = vec3{155, 118, 83} / 255.0;

  plane_color = lerp(plane_color, bg_color, r.at(plane_time).length() / 100.0);
  if (plane_time > 100) {
    plane_color = bg_color;
  }

  if (!final.hit && plane_time <= 0) {
    return bg_color;
  }

  if (!final.hit || plane_time > 0 && plane_time < final.t) {
    vec3 p = r.at(plane_time);
    vec3 reflect = r.direction - 2 * dot(r.direction, {0, 1, 0}) * vec3({0, 1, 0});
    // Reflect ray up if hits plane
    vec3 res = shoot_ray(Ray{p + 0.001 * vec3{0, 1, 0}, reflect}, bvh_root);
    if (res == bg_color) {
      return plane_color;
    }
    // Make reflection dimmer
    return lerp(res, plane_color, 0.7);
  }

  if (!final.hit) {
    return {0, 0, 0};
  }

  // Diffuse and specular
  vec3 light = {10, 10, 10};
  vec3 to_light = unit_vector(light - final.point);
  double diffuse = std::max(dot(to_light, final.normal), 0.0);
  vec3 reflect = unit_vector(r.direction - 2 * dot(r.direction, final.normal) * final.normal);

  // Cap out colors at [0, 1] so doesnt go out of bounds
  return min_e(vec3{1, 1, 1}, max_e(vec3{}, diffuse * final.albedo + vec3{1, 1, 1} * std::pow(std::max(dot(reflect, to_light), 0.0), 8)));
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
  const size_t width = 1200;
  const size_t height = 1200;
  const size_t channels = 3;
  char png[height][width][channels] = {};

  // Switch this if needed
  bool is_ortho = false;

  // For spheres
  //vec3 camera_pos = {0, 0, 2};
  //vec3 camera_forward = unit_vector(vec3(0, 0, -100) - camera_pos);

  // For mesh
  vec3 camera_pos = {0.4, 0.3, 2.5};
  vec3 camera_forward = unit_vector(vec3(0.4, 0, 0) - camera_pos);

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
  double focal = 1.0;

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
  size_t n = 1;
  double n_d = n;
  Sample samples[n][n] = {};

  std::vector<std::unique_ptr<Hittable>> world;

  // Uncomment to generate sphere scene, and dont forget to change camera coordinates to see it
  /*
  for (size_t i = 0; i < 10000; ++i) {
    world.push_back(std::make_unique<Sphere>(
      vec3{(
        rand_int(0, 1000) / 1000.0 - 0.5) * 6, // x
        (rand_int(0, 1000) / 1000.0 - 0.5) * 6, // y
        -5 - 1 * rand_int(0, 1000) / 1000.0}, // z
      0.015, // radius
      vec3(rand_int(0, 1000) / 1000.0, rand_int(0, 1000) / 1000.0, rand_int(0, 1000) / 1000.0) // color
    ));
  }*/

  // Tiny obj loader reference code
  std::string inputfile = "cow.obj";
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string err;

  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str(), "./", false);

  if (!err.empty()) {
    std::cerr << err << std::endl;
  }

  if (!ret) {
    exit(1);
  }

  for (size_t s = 0; s < shapes.size(); s++) {
    // First loop through - generate normals
    std::unordered_map<int, vec3> normals;

    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      // Convert vertices to vec3
      vec3 vertices[3];

      for (size_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
        tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
        tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];

        vertices[v] = vec3(vx, vy, vz);
      }

      // Compute normal
      vec3 e1 = vertices[0] - vertices[1];
      vec3 e2 = vertices[2] - vertices[1];
      vec3 normal = cross(e2, e1);

      // Add normal to all vertices touching
      for (size_t v = 0; v < fv; v++) {
        int idx = shapes[s].mesh.indices[index_offset + v].vertex_index;
        if (normals.find(idx) == normals.end()) {
          normals[idx] = {};
        }

        normals[idx] += normal;
      }

      index_offset += fv;

      // per-face material
      shapes[s].mesh.material_ids[f];
    }

    // Second loop create triangles
    index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      vec3 vertices[3];
      vec3 cur_normals[3];
      
      for (size_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
        tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
        tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];

        vertices[v] = vec3(vx, vy, vz);
        cur_normals[v] = unit_vector(normals[idx.vertex_index]);
      }

      // Create triangle here
      world.push_back(std::make_unique<Triangle>(vertices[0], vertices[1], vertices[2], cur_normals[0], cur_normals[1], cur_normals[2]));

      index_offset += fv;

      // per-face material
      shapes[s].mesh.material_ids[f];
    }
  }

  // Creates grass
  for (size_t i = 0; i < 7; ++i)
  for (size_t j = 0; j < 5; ++j)
  world.push_back(std::make_unique<Sphere>(
    vec3{0.9, -0.8, -0.4} + vec3{1, 0, 0} * i * 0.15 + vec3{0, 0, 1} * j * 0.15,
    0.3, // radius
    vec3{126, 200, 80} / 255.0 // color
  ));

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
          color_sum += shoot_ray(ray, bvh_root);
        }
      }

      // Assign final color
      img_assign(png[r][c], color_sum / (n * n));
    }
  }
  
  // Write image
  stbi_write_png("out/test.png", width, height, channels, png, width * channels);
  return 0;
}

// Uncomment and rename other main() to main2 to run with SDL
/*
// https://gist.github.com/CoryBloyd/6725bb78323bb1157ff8d4175d42d789
#include <stdio.h>
#include <SDL2/SDL.h>
inline uint32_t argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) { return (a<<24) | (r << 16) | (g << 8) | (b << 0); }

int main2(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Rect    screenRect = { 0,0,500, 500 };
    SDL_Window   *window   = SDL_CreateWindow("SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenRect.w, screenRect.h, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture  *texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screenRect.w, screenRect.h);
    
    for (int frame = 0; ; ++frame) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
            if (event.type == SDL_QUIT)
                return 0;
    
        int pitch;
        int *pixels;
        SDL_LockTexture(texture, &screenRect, (void**)&pixels, &pitch);
        uint32_t startTicks = SDL_GetTicks();
        const size_t width = 500;
        const size_t height = 500;
        const size_t channels = 3;

        bool is_ortho = false;

        vec3 camera_pos = {4 * std::sin(frame / 20.0), 1, 4 * std::cos(frame / 20.0)};
        vec3 camera_forward = unit_vector(vec3(0, 0.5, -2) - camera_pos);

        if (is_ortho) {
          camera_pos = {4 * std::sin(frame / 20.0), 2, 4 * std::cos(frame / 20.0)};
          camera_forward = unit_vector(vec3(0, 1, -2) - camera_pos);
        }

        vec3 camera_right = cross(camera_forward, {0, 1, 0});
        vec3 camera_up = cross(camera_right, camera_forward);

        double aspect_ratio = static_cast<double>(width) / height;
        double focal = 1.0;

        double viewport_height = 1;
        if (is_ortho) {
          viewport_height *= 3;
        }
        double viewport_width = viewport_height * aspect_ratio;
        vec3 viewport_right = viewport_width * camera_right;
        vec3 viewport_down = -viewport_height * camera_up;
        
        vec3 viewport_top_left = camera_pos - viewport_right / 2 - viewport_down / 2 + focal * camera_forward;

        for (size_t r = 0; r < height; ++r) {
          for (size_t c = 0; c < width; ++c) {
            double row_ratio = (static_cast<double>(r) + 0.5) / height;
            double col_ratio = (static_cast<double>(c) + 0.5) / width;
            Ray ray = {camera_pos, viewport_top_left + viewport_down * row_ratio + viewport_right * col_ratio - camera_pos};
            if (is_ortho) {
              ray = {viewport_top_left + viewport_down * row_ratio + viewport_right * col_ratio, camera_forward};
            }
            char p[4];
            img_assign(p, shoot_ray(ray));
            p[3] = 255;
            pixels[r*screenRect.w + c] = argb(p[3], p[0], p[1], p[2]);
          }
        }
        uint32_t endTicks = SDL_GetTicks();
        SDL_UnlockTexture(texture);

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, &screenRect, &screenRect);
        SDL_RenderPresent(renderer);

        char title[32];
        SDL_SetWindowTitle(window, SDL_itoa(endTicks - startTicks, title, 10));
    }
}*/
