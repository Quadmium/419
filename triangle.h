#ifndef TRIANGLE_H_
#define TRIANGLE_H_
#include "hittable.h"

class Triangle : public Hittable {
  public:
    // Triangle has three vertices and a normal per-vertex
    Triangle(const vec3 &vertex0, const vec3 &vertex1, const vec3 &vertex2, const vec3 &normal0, const vec3 &normal1, const vec3 &normal2) : vertex0(vertex0), vertex1(vertex1), vertex2(vertex2), normal0(normal0), normal1(normal1), normal2(normal2) {
      vec3 e1 = vertex0 - vertex1;
      vec3 e2 = vertex2 - vertex1;
      normal = unit_vector(cross(e2, e1));
    }

    // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    // Does ray r hit triangle between t_min and t_max
    virtual HitResult hit(const Ray& r, double t_min, double t_max) {
      HitResult res = {};
      
      const float EPSILON = 0.0000001;
      vec3 edge1, edge2, h, s, q;
      float a,f,u,v;
      edge1 = vertex1 - vertex0;
      edge2 = vertex2 - vertex0;
      h = cross(r.direction, edge2);
      a = dot(edge1, h);
      if (a > -EPSILON && a < EPSILON)
          return res;    // This ray is parallel to this triangle.
      f = 1.0/a;
      s = r.origin - vertex0;
      u = f * dot(s, h);
      if (u < 0.0 || u > 1.0)
          return res;
      q = cross(s, edge1);
      v = f * dot(r.direction, q);
      if (v < 0.0 || u + v > 1.0)
          return res;
      // At this stage we can compute t to find out where the intersection point is on the line.
      float t = f * dot(edge2, q);
      if (t > EPSILON && t >= t_min && t <= t_max) // ray intersection
      {
        res.t = t;
      }
      else // This means that there is a line intersection but not a ray intersection.
      {
        return res;
      }

      res.hit = true;
      res.point = r.at(res.t);
      res.normal = (1 - u - v) * normal0 + u * normal1 + v * normal2;
      res.albedo = vec3(1, 215/255.0, 0);

      return res;
    }

    // Bounding box of triangle
    virtual AABB aabb() {
      return {
        vec3(std::min(std::min(vertex0.x(), vertex1.x()), vertex2.x()), std::min(std::min(vertex0.y(), vertex1.y()), vertex2.y()), std::min(std::min(vertex0.z(), vertex1.z()), vertex2.z())),
        vec3(std::max(std::max(vertex0.x(), vertex1.x()), vertex2.x()), std::max(std::max(vertex0.y(), vertex1.y()), vertex2.y()), std::max(std::max(vertex0.z(), vertex1.z()), vertex2.z()))
      };
    }
  
    vec3 vertex0;
    vec3 vertex1;
    vec3 vertex2;
    vec3 normal0;
    vec3 normal1;
    vec3 normal2;
    vec3 normal;
};

#endif