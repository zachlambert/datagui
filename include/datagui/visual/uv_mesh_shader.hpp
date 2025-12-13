#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/geometry/rot.hpp"
#include <vector>

namespace datagui {

class Mesh {
public:
  Mesh() : VBO(0), EBO(0) {}
  ~Mesh();
  Mesh(Mesh&&);

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;
  Mesh& operator=(Mesh&&) = delete;

  void load_vertices(
      float* const positions,
      float* const normals,
      std::size_t num_vertices,
      std::size_t stride);

  void load_indices(unsigned int* const indices, std::size_t num_indices);

private:
  unsigned int VBO;
  unsigned int EBO;
};

class UvMesh {
public:
  UvMesh() : VBO(0), EBO(0), texture(0) {}
  ~UvMesh();
  UvMesh(UvMesh&&);

  UvMesh(const Mesh&) = delete;
  UvMesh& operator=(const Mesh&) = delete;
  UvMesh& operator=(Mesh&&) = delete;

  void load_vertices(
      float* const positions,
      float* const normals,
      float* const uv,
      std::size_t num_vertices,
      std::size_t stride);

  void load_indices_uint(unsigned int* const indices, std::size_t num_indices);

  void load_texture(
      std::uint8_t* const pixels,
      std::size_t width,
      std::size_t height);

private:
  unsigned int VBO;
  unsigned int EBO;
  unsigned int texture;
};

class MeshShader {
public:
  void init();

  void queue_mesh(
      const Vec3& position,
      const Rot3& orientation,
      const Mesh& mesh);

  void queue_uv_mesh(
      const Vec3& position,
      const Rot3& orientation,
      const UvMesh& mesh);

  void draw(const Vec2& viewport_size, const Camera3d& camera);
  void clear();

private:
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
