Worked with curieh2

Find report in report.pdf
Find images in out/*.png

To run, rename main_X.cpp to main.cpp. 3 files provided, one per scene.

out/mirror shows a mirror ground with a beach ball and a solid sphere, and marbles scattered around.
out/shadow shows an area light with 3 spheres and no other light source.
out/transparent shows a hollow glass sphere on a checkerboard ground.

Code attributions:
The following parts were based on https://raytracing.github.io/books/RayTracingInOneWeekend.html:
AABB.h hit algorithm
BVH.h structure of BVH class
hittable.h idea of hittable struct and normal with direction
material.h ideas for implementation of Lambert / Metal / Dielectric
rectangle.h rectangle hit algorithm
vec3.h vector utility library

Used https://github.com/nothings/stb/blob/master/stb_image_write.h for png
Used https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
Used // https://gist.github.com/CoryBloyd/6725bb78323bb1157ff8d4175d42d789 for SDL2 setup

Consulted https://cs.dartmouth.edu/wjarosz/publications/subr16fourier-slides-2-patterns.pdf for multi jittering explanation