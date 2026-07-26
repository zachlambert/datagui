#include "datagui/render/program/shape_3d_program.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/geometry/rot.hpp"
#include "datagui/visual/window.hpp"
#include <GL/glew.h>
#include <array>
#include <cmath>
#include <vector>

int main() {
  using namespace dgui;

  Window window;
  window.open("shape_3d_program", 800, 800);

  Shape3dProgram program;
  program.init();

  Camera3d camera;
  camera.direction.x = std::cos(M_PI / 4);
  camera.direction.z = -std::sin(M_PI / 4);
  camera.position.x = -8.0;
  camera.position.z = 6;
  camera.fov = Vec2::uniform(M_PI / 2);
  camera.clipping_min = 0.001;
  camera.clipping_max = 1000;

  std::array<std::vector<Shape3dInstance>, Shape3dTypeCount> instances;
  auto add = [&](Shape3dType type, const Shape3dInstance& instance) {
    instances[(std::size_t)type].push_back(instance);
  };

  add(Shape3dType::Box,
      Shape3dInstance::box(
          Vec3(),
          Euler(M_PI / 4, 0, 0),
          Vec3::uniform(1),
          Color::Red()));
  add(Shape3dType::Cylinder,
      Shape3dInstance::cylinder(
          Vec3(0, 2, 0),
          Vec3(0, 0, 1),
          0.5,
          2,
          Color::Green()));
  add(Shape3dType::Sphere,
      Shape3dInstance::sphere(Vec3(0, -2, 0), 1.5, Color::Blue()));
  add(Shape3dType::Cone,
      Shape3dInstance::cone(
          Vec3(-3, 3, 0),
          Vec3(0, 0, 1),
          1,
          5,
          Color::Hsl(180, 1, 0.5)));
  add(Shape3dType::HalfSphere,
      Shape3dInstance::half_sphere(
          Vec3(-3, -3, 0),
          Vec3(0, 0, 1),
          1,
          Color::Hsl(300, 1, 0.5)));
  add(Shape3dType::Plane,
      Shape3dInstance::plane(
          Vec3(0, 0, -1),
          Rot3(),
          Vec2(10, 10),
          Color::Hsl(150, 0.3, 0.8)));

  auto capsule = CapsuleInstance::make(
      Vec3(-2, -4, 2),
      Vec3(-1, -4, 3),
      0.5,
      Color::Gray(0.5));
  add(Shape3dType::Cylinder, capsule.cylinder);
  add(Shape3dType::HalfSphere, capsule.half_sphere_1);
  add(Shape3dType::HalfSphere, capsule.half_sphere_2);

  auto arrow = ArrowInstance::make(
      Vec3(-4, -2, 0),
      Vec3(-2, 2, 0),
      0.4,
      Color::Gray(0.8));
  add(Shape3dType::Cylinder, arrow.cylinder);
  add(Shape3dType::Cone, arrow.cone);

  while (window.running()) {
    window.render_begin();

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    Mat4 V = camera.view_mat();
    Mat4 P = camera.projection_mat();

    program.bind();
    for (std::size_t i = 0; i < Shape3dTypeCount; i++) {
      const auto& list = instances[i];
      if (!list.empty()) {
        program.draw((Shape3dType)i, list.data(), list.size(), P, V);
      }
    }

    window.render_end();
    window.poll_events();
  }
}
