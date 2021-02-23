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

// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
double hit_triangle(const Ray &r, const vec3 &vertex0, const vec3 &vertex1, const vec3 &vertex2) {
    const float EPSILON = 0.0000001;
    vec3 edge1, edge2, h, s, q;
    float a,f,u,v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = cross(r.direction, edge2);
    a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return -1;    // This ray is parallel to this triangle.
    f = 1.0/a;
    s = r.origin - vertex0;
    u = f * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return -1;
    q = cross(s, edge1);
    v = f * dot(r.direction, q);
    if (v < 0.0 || u + v > 1.0)
        return -1;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dot(edge2, q);
    if (t > EPSILON) // ray intersection
    {
        return t;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return -1;
}


vec3 shoot_ray(const Ray &r, bool *hit=nullptr) {
  vec3 anchor = {0, 0, 0};
  vec3 normal = {0, 1, 0};
  vec3 light = {10, 10, 10};

  double plane_hit_time = hit_plane(anchor, normal, r);

  vec3 v0 = {0.5, 0.5, -1};
  vec3 v1 = {1.5, 0.5, -1};
  vec3 v2 = {1.5, 1.5, -2};

  double triangle_hit_time = hit_triangle(r, v0, v1, v2);

  double t0, t1;
  if (hit_sphere({0, 0.5, -2}, 0.5, r, &t0, &t1) && (plane_hit_time <= 0 || t0 <= plane_hit_time) && (triangle_hit_time <= 0 || t0 <= triangle_hit_time)) {
    if (hit != nullptr) {
      *hit = true;
      return {0, 0, 0};
    }
    vec3 normal = unit_vector(r.at(t0) - vec3(0,0.5,-2));
    vec3 sphere_hit = r.at(t0);
    vec3 to_light = unit_vector(light - sphere_hit);
    double diffuse = std::max(dot(to_light, normal), 0.0);
    bool hit_shadow;
    shoot_ray({sphere_hit + normal * 0.001, to_light}, &hit_shadow);
    if (hit_shadow) {
      return {0, 0, 0};
    }
    return diffuse * vec3(0, 0.8, 0.8);
  }

  if (triangle_hit_time > 0 && (plane_hit_time <= 0 || triangle_hit_time <= plane_hit_time)) {
    if (hit != nullptr) {
      *hit = true;
      return {0, 0, 0};
    }

    vec3 e1 = v0 - v1;
    vec3 e2 = v2 - v1;

    vec3 tri_normal = unit_vector(cross(e2, e1));
    vec3 tri_hit = r.at(triangle_hit_time);
    vec3 to_light = unit_vector(light - tri_hit);
    double diffuse = std::max(dot(to_light, normal), 0.0);

    return diffuse * vec3(1, 1, 1);
  }

  if (plane_hit_time > 0) {
    if (hit != nullptr) {
      *hit = true;
      return {0, 0, 0};
    }
    vec3 plane_hit = r.at(plane_hit_time);
    vec3 to_light = unit_vector(light - plane_hit);
    double diffuse = std::max(dot(to_light, normal), 0.0);

    bool hit_shadow;
    shoot_ray({plane_hit + normal * 0.001, to_light}, &hit_shadow);
    if (hit_shadow) {
      return {0, 0, 0};
    }
    return diffuse * vec3(1, 1, 1);
  }

  if (hit != nullptr) {
    *hit = false;
  }
  return {0, 0, 0};
}

int rand_int(int min, int max) {
  return rand() % (max - min + 1) + min;
}

struct Sample {
  double r;
  double c;
};

int main(int argc, char **argv) {
  const size_t width = 300;
  const size_t height = 300;
  const size_t channels = 3;
  char png[height][width][channels] = {};

  bool is_ortho = false;

  int frame = 0;
  vec3 camera_pos = {2 * std::sin(frame / 20.0), 1, 2 * std::cos(frame / 20.0)};
  vec3 camera_forward = unit_vector(vec3(0, 0.5, -2) - camera_pos);

  if (is_ortho) {
    camera_pos = {4 * std::sin(0 / 20.0), 2, 4 * std::cos(0 / 20.0)};
    camera_forward = unit_vector(vec3(0, 1, -2) - camera_pos);
  }

  vec3 camera_right = cross(camera_forward, {0, 1, 0});
  vec3 camera_up = cross(camera_right, camera_forward);

  double aspect_ratio = static_cast<double>(width) / height;
  double focal = 1.0;

  double viewport_height = 1;
  if (is_ortho) {
    viewport_height *= 3.5;
  }

  double viewport_width = viewport_height * aspect_ratio;
  vec3 viewport_right = viewport_width * camera_right;
  vec3 viewport_down = -viewport_height * camera_up;

  vec3 viewport_top_left = camera_pos - viewport_right / 2 - viewport_down / 2 + focal * camera_forward;

  size_t n = 1;
  double n_d = n;
  Sample samples[n][n] = {};

  for (size_t r = 0; r < height; ++r) {
    for (size_t c = 0; c < width; ++c) {
      for (size_t r = 0; r < n; ++r) {
        for (size_t c = 0; c < n; ++c) {
          samples[r][c].r = r / n_d + (c % n) / n_d / n_d + 0.5 / n_d / n_d;
          samples[r][c].c = c / n_d + (r % n) / n_d / n_d + 0.5 / n_d / n_d;
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

      vec3 color_sum = {};

      for (size_t r_s = 0; r_s < n; ++r_s) {
        for (size_t c_s = 0; c_s < n; ++c_s) {
          double row_ratio = (static_cast<double>(r) + samples[r_s][c_s].r) / height;
          double col_ratio = (static_cast<double>(c) + samples[r_s][c_s].c) / width;
          Ray ray = {camera_pos, viewport_top_left + viewport_down * row_ratio + viewport_right * col_ratio - camera_pos};
          if (is_ortho) {
            ray = {viewport_top_left + viewport_down * row_ratio + viewport_right * col_ratio, camera_forward};
          }
          color_sum += shoot_ray(ray);
        }
      }

      img_assign(png[r][c], color_sum / (n * n));
    }
  }
  
  stbi_write_png("out/test.png", width, height, channels, png, width * channels);
  return 0;
}

// Uncomment and rename other main() to main2 to run with SDL
/*
// https://gist.github.com/CoryBloyd/6725bb78323bb1157ff8d4175d42d789
#include <stdio.h>
#include <SDL2/SDL.h>
inline uint32_t argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) { return (a<<24) | (r << 16) | (g << 8) | (b << 0); }

int main(int argc, char *argv[]) {
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
}
*/