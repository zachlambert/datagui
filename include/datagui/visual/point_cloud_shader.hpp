#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/geometry/rot.hpp"
#include <vector>

namespace datagui {

class PointCloud {
public:
  PointCloud() : initialized(false), VAO(0), VBO(0), vertex_count(0) {}
  ~PointCloud();
  PointCloud(PointCloud&&);

  PointCloud(const PointCloud&) = delete;
  PointCloud& operator=(const PointCloud&) = delete;
  PointCloud& operator=(PointCloud&&) = delete;

  void load_points(
      void* points,
      std::size_t num_points,
      std::size_t positions_offset,
      std::size_t colors_offset,
      std::size_t stride);

private:
  void init();

  struct Vertex {
    Vec3 position;
    Vec3 color;
  };

  bool initialized;
  unsigned int VAO;
  unsigned int VBO;
  std::size_t vertex_count;

  friend class PointCloudShader;
};

class PointCloudShader {
public:
  void init();

  void queue_point_cloud(
      const PointCloud& point_cloud,
      const Vec3& position,
      const Rot3& orientation,
      float point_size);

  void draw(const Vec2& viewport_size, const Camera3d& camera);
  void clear();

private:
  struct Command {
    unsigned int VAO;
    std::size_t vertex_count;
    Mat4 model_mat;
    float point_size;
  };
  std::vector<Command> commands;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_P;
  unsigned int uniform_V;
  unsigned int uniform_M;
  unsigned int uniform_point_size;
};

} // namespace datagui
