#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry/box.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/geometry/rot.hpp"
#include <vector>

namespace datagui {

class Shape3dShader {
public:
  void init();

  void queue_box(
      const Vec3& position,
      const Rot3& orientation,
      const Vec3& size,
      const Color& color);

  void queue_cylinder(
      const Vec3& base_position,
      const Vec3& direction,
      float radius,
      float length,
      const Color& color);

  void queue_sphere(const Vec3& position, float radius, const Color& color);

  void queue_half_sphere(
      const Vec3& position,
      const Vec3& direction,
      float radius,
      const Color& color);

  void queue_cone(
      const Vec3& base_position,
      const Vec3& direction,
      float radius,
      float length,
      const Color& color);

  void queue_capsule(
      const Vec3& start,
      const Vec3& end,
      float radius,
      const Color& color);

  void queue_arrow(
      const Vec3& start,
      const Vec3& end,
      float radius,
      const Color& color,
      float head_length_scale = 2,
      float head_radius_scale = 2);

  void queue_plane(
      const Vec3& position,
      const Rot3& orientation,
      const Vec2& scale,
      const Color& color);

  void draw(const Box2& viewport, const Camera3d& camera);
  void clear();

private:
  enum class ShapeType {
    Box,
    Cylinder,
    Sphere,
    HalfSphere,
    Cone,
    Plane,
  };
  static constexpr std::size_t ShapeTypeCount = 6;

  struct Shape {
    std::size_t indices_begin;
    std::size_t indices_end;
  };
  std::vector<Shape> shapes;

  struct Element {
    Mat4 transform;
    Color color;
  };
  std::vector<std::vector<Element>> elements;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_P;
  unsigned int uniform_V;

  // Array/buffer objects
  unsigned int VAO;
  unsigned int static_VBO;
  unsigned int static_EBO;
  unsigned int instance_VBO;
};

} // namespace datagui
