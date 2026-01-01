#pragma once

#include "datagui/asset/point_cloud.hpp"
#include "datagui/geometry/box.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/geometry/rot.hpp"
#include <vector>

namespace datagui {

class PointCloudShader {
public:
  void init();

  void queue_point_cloud(
      const PointCloud& point_cloud,
      const Vec3& position,
      const Rot3& orientation,
      float point_size);

  void draw(const Box2& viewport, const Camera3d& camera);
  void clear();

private:
  struct Command {
    PointCloud point_cloud;
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
