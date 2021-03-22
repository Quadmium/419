#ifndef BVH_H_
#define BVH_H_
#include <memory>
#include <vector>
#include <cassert>
#include <algorithm>
#include "hittable.h"

// BVH Tree
class BVHNode : Hittable {
  public:
    // A list of hittable objects to generate node for
    BVHNode(std::vector<Hittable*> objects) {
      assert(objects.size() > 0);

      // Leaf node has merged box as its box
      if (objects.size() <= 2) {
        contents = objects;
        aabb_box = contents[0]->aabb();
        for (size_t i = 1; i < contents.size(); ++i) {
          aabb_box = aabb_box.merge(contents[i]->aabb());
        }
        return;
      }

      // Figure out min and max of centers
      vec3 min = objects[0]->aabb().center;
      vec3 max = objects[0]->aabb().center;

      for (Hittable *h : objects) {
        min = min_e(min, h->aabb().center);
        max = max_e(max, h->aabb().center);
      }

      // Figure out max spread axis
      vec3 spread = max - min;
      int axis = 0;
      double max_spread = -1;
      for (int i = 0; i < 3; ++i) {
        if (spread[i] > max_spread) {
          max_spread = spread[i];
          axis = i;
        }
      }

      // Sort the objects along max axis
      std::sort(objects.begin(), objects.end(), [axis](Hittable *lhs, Hittable *rhs) {
        return lhs->aabb().center[axis] < rhs->aabb().center[axis];
      });

      auto mid = objects.begin() + objects.size() / 2;

      left = std::make_unique<BVHNode>(std::vector<Hittable*>(objects.begin(), mid));
      right = std::make_unique<BVHNode>(std::vector<Hittable*>(mid, objects.end()));

      aabb_box = left->aabb().merge(right->aabb());
    }

    // Does ray r hit any object in this tree between t_min and t_max
    virtual HitResult hit(const Ray &r, double t_min, double t_max) {
      // Overall check
      if (!aabb_box.hit(r, t_min, t_max)) {
        return {};
      }

      // Inner node
      if (contents.empty()) {
        HitResult left_hit = left->hit(r, t_min, t_max);
        HitResult right_hit = right->hit(r, t_min, t_max);
        HitResult res = {};

        if (left_hit.hit && right_hit.hit) {
          res = left_hit.t <= right_hit.t ? left_hit : right_hit;
        } else if (left_hit.hit) {
          res = left_hit;
        } else if (right_hit.hit) {
          res = right_hit;
        }

        return res;
      }

      // Leaf node
      HitResult res = {};
      for (Hittable *h : contents) {
        HitResult h_res = h->hit(r, t_min, t_max);
        if (h_res.hit) {
          if (!res.hit || h_res.t < res.t) {
            res = h_res;
          }
        }
      }

      return res;
    }

    // Bounding box of tree node
    virtual AABB aabb() {
      return aabb_box;
    }

    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;

    std::vector<Hittable*> contents;

    AABB aabb_box;
};

#endif