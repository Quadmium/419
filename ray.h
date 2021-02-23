#ifndef RAY_H_
#define RAY_H_
#include "vec3.h"

// Represents a ray with origin and direction
struct Ray {
  vec3 origin;
  vec3 direction;
  Ray() : origin(vec3()), direction(vec3()) {}

  Ray(vec3 origin, vec3 direction) : origin(std::move(origin)), direction(std::move(direction)) {}

  // Calculates R(t)
  vec3 at(double t) const {
    return origin + direction * t;
  }
};

#endif