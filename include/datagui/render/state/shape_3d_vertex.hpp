#pragma once

#include "datagui/geometry/vec.hpp"
#include "datagui/render/state/shape_3d_instance.hpp"
#include <vector>

namespace dgui {

struct Shape3dVertex {
  Vec3 position;
  Vec3 normal;
};

void add_shape_vertices(
    Shape3dType type,
    std::vector<Shape3dVertex>& vertices,
    std::vector<unsigned int>& indices);

} // namespace dgui
