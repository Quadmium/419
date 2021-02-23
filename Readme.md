Worked with curieh2

Find images in out/*.png
orthographic, perspective, perspective_viewpoint (another angle), no_multi_jitter (anti aliasing off), multi_jitter (anti aliasing on)

To change camera viewpoint / orthographic, change the following in main.cpp:
  // Switch this if needed
  bool is_ortho = false;

  int frame = 0;
  // Change this to any vectors if needed
  vec3 camera_pos = {2 * std::sin(frame / 20.0), 1, 2 * std::cos(frame / 20.0)};
  vec3 camera_forward = unit_vector(vec3(0, 0.5, -2) - camera_pos);

Can run in SDL2 to see realtime orbit (see commented code at bottom of main.cpp)
Can see this in out/sdl2.mp4

Used https://github.com/nothings/stb/blob/master/stb_image_write.h for png
Used https://raytracing.github.io/books/RayTracingInOneWeekend.html for vec3
Used https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
Used // https://gist.github.com/CoryBloyd/6725bb78323bb1157ff8d4175d42d789 for SDL2 setup

Consulted https://cs.dartmouth.edu/wjarosz/publications/subr16fourier-slides-2-patterns.pdf for multi jittering explanation