#pragma once

#include "datagui/render/state/glyph_2d_vertex.hpp"
#include "datagui/render/state/shape_2d_instance.hpp"
#include <map>

namespace dgui {

struct DrawList {
  std::vector<Shape2dInstance> shape_2d_instances;
  std::vector<Glyph2dVertex> glyph_2d_vertices;

  struct BatchGui {
  };

  struct GlyphGroup {
    unsigned int font_texture;
    Color color;
    size_t offset = 0;
    size_t count = 0;
    bool matches(int font_texture, const Color& color) {
      return font_texture == this->font_texture && color.equals(this->color);
    }
  };

  struct ImageGroup {
    Image image;
    size_t offset = 0;
    size_t count = 0;
  };

  struct Batch {
    Box2 mask;
    size_t shape_offset = 0;
    size_t shape_count = 0;
    std::vector<GlyphGroup> glyph_groups;
    std::vector<ImageGroup> image_groups;
    const Scene3d* scene_3d = nullptr;
  };
  std::map<int, std::vector<Batch>> batches;
};

} // namespace dgui
