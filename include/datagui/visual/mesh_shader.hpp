#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry/box.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/geometry/rot.hpp"
#include <vector>

namespace datagui {

class Mesh {
public:
  Mesh() : initialized(false), VAO(0), VBO(0), EBO(0), index_count(0) {}
  ~Mesh();
  Mesh(Mesh&&);

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;
  Mesh& operator=(Mesh&&) = delete;

  void load_vertices(
      void* vertices,
      std::size_t num_vertices,
      std::size_t positions_offset,
      std::size_t normals_offset,
      std::size_t stride);

  void load_indices(unsigned int* const indices, std::size_t num_indices);

private:
  struct Vertex {
    Vec3 position;
    Vec3 normal;
  };

  void init();
  bool initialized;

  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
  std::size_t index_count;

  friend class MeshShader;
};

class MeshShader {
public:
  void init();

  void queue_mesh(
      const Mesh& mesh,
      const Vec3& position,
      const Rot3& orientation,
      const Color& color);

  void draw(const Box2& viewport, const Camera3d& camera);
  void clear();

private:
  struct Command {
    unsigned int VAO;
    std::size_t index_count;
    Mat4 model_mat;
    Color color;
  };
  std::vector<Command> commands;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_P;
  unsigned int uniform_V;
  unsigned int uniform_M;
  unsigned int uniform_mesh_color;
};

} // namespace datagui
