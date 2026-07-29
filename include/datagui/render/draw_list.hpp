#pragma once

#include "datagui/asset/image.hpp"
#include "datagui/asset/mesh.hpp"
#include "datagui/asset/point_cloud.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/render/state/glyph_2d_instance.hpp"
#include "datagui/render/state/shape_2d_instance.hpp"
#include "datagui/render/state/shape_3d_instance.hpp"
#include <variant>

namespace dgui {

struct DrawList {
  std::vector<Shape2dInstance> shape_2d_instances;
  std::vector<Glyph2dInstance> glyph_2d_instances;
  std::array<std::vector<Shape3dInstance>, Shape3dTypeCount> shape_3d_instances;

  struct BeginGroup {
    Box2 mask;
  };
  struct BeginScene2d {
    Box2 viewport;
    Camera2d camera;
  };
  struct BeginScene3d {
    Box2 viewport;
    Camera3d camera;
  };

  struct DrawShape2d {
    size_t offset;
    size_t count;
  };
  struct DrawGlyph2d {
    unsigned int font_texture;
    size_t offset;
    size_t count;
  };
  struct DrawImage2d {
    Image image;
    Mat3 transform;
  };

  struct DrawShape3d {
    Shape3dType type;
    size_t offset;
    size_t count;
  };
  struct DrawMesh {
    Mesh mesh;
    Mat4 transform;
    Color base_color;
  };
  struct DrawPointCloud {
    PointCloud point_cloud;
    Mat4 transform;
    float point_size;
    Color base_color;
  };

  using Action = std::variant<
      BeginGroup,
      BeginScene2d,
      BeginScene3d,
      DrawShape2d,
      DrawGlyph2d,
      DrawImage2d,
      DrawShape3d,
      DrawMesh,
      DrawPointCloud>;
  std::vector<Action> actions;
};

} // namespace dgui
