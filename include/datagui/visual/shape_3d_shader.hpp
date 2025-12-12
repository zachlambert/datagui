#pragma once

#include "datagui/color.hpp"
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

  void draw(const Vec2& viewport_size, const Camera3d& camera);
  void clear();

private:
  enum class ShapeType {
    Box,
  };
  static constexpr std::size_t ShapeTypeCount = 1;

  struct Shape {
    std::size_t indices_offset;
    std::size_t index_count;
  };
  std::vector<Shape> shapes;

  struct Element {
    Mat4 transform;
    Color color;
  };
  std::vector<std::vector<Element>> elements;

  struct Args {
    float ambient = 0.4;
  };
  Args args;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_P;
  unsigned int uniform_V;
  unsigned int uniform_ambient;

  // Array/buffer objects
  unsigned int VAO;
  unsigned int static_VBO;
  unsigned int static_EBO;
  unsigned int instance_VBO;
};

} // namespace datagui
