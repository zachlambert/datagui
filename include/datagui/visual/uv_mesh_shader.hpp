#pragma once

#include "datagui/geometry/box.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/geometry/rot.hpp"
#include <vector>

namespace datagui {

class UvMesh {
public:
  UvMesh() : initialized(false), VAO(0), VBO(0), EBO(0), index_count(0) {}
  ~UvMesh();
  UvMesh(UvMesh&&);

  UvMesh(const UvMesh&) = delete;
  UvMesh& operator=(const UvMesh&) = delete;
  UvMesh& operator=(UvMesh&&) = delete;

  void load_vertices(
      void* vertices,
      std::size_t num_vertices,
      std::size_t positions_offset,
      std::size_t normals_offset,
      std::size_t uv_offset,
      std::size_t stride);

  void load_indices(unsigned int* const indices, std::size_t num_indices);

  void load_texture(std::size_t width, std::size_t height, void* data);

private:
  struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 uv;
  };

  void init();
  bool initialized;

  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
  unsigned int texture;
  std::size_t index_count;

  friend class UvMeshShader;
};

class UvMeshShader {
public:
  void init();

  void queue_mesh(
      const UvMesh& mesh,
      const Vec3& position,
      const Rot3& orientation,
      float opacity = 1);

  void draw(const Box2& viewport, const Camera3d& camera);
  void clear();

private:
  struct Command {
    unsigned int VAO;
    unsigned int texture;
    std::size_t index_count;
    Mat4 model_mat;
    float opacity;
  };
  std::vector<Command> commands;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_P;
  unsigned int uniform_V;
  unsigned int uniform_M;
  unsigned int uniform_opacity;
};

} // namespace datagui
