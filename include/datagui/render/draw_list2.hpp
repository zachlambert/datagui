#pragma once

// IGNORE THIS FILE - experimental

#include "datagui/layout.hpp"
#include "datagui/render/font_atlas.hpp"
#include "datagui/render/state/glyph_2d_instance.hpp"
#include "datagui/render/state/shape_2d_instance.hpp"
#include "datagui/render/state/shape_3d_instance.hpp"
#include <map>
#include <optional>

namespace dgui {

struct DrawList {
  struct Glyph2dGroup {
    unsigned int font_texture;
    Color color;
    size_t offset = 0;
    size_t count = 0;
    bool matches(int font_texture, const Color& color) {
      return font_texture == this->font_texture && color.equals(this->color);
    }
    Glyph2dGroup(unsigned int font_texture, const Color& color) :
        font_texture(font_texture), color(color) {}
  };
  struct Image {
    unsigned int image_texture;
    Mat3 M;
  };

  std::vector<Shape2dInstance> shape_2d_instances;
  std::vector<Glyph2dInstance> glyph_2d_vertices;
  std::vector<Glyph2dGroup> glyph_2d_groups;
  std::vector<Image> images;

  std::array<std::vector<Shape3dInstance>, Shape3dTypeCount> shape_3d_instances;

  struct Batch {
    Box2 mask;
    size_t shape_2d_offset = 0;
    size_t shape_2d_count = 0;
    size_t glyph_2d_group_offset = 0;
    size_t glyph_2d_group_count = 0;
    size_t image_offset = 0;
    size_t image_count = 0;
    Batch(const Box2& mask) : mask(mask) {}
  };
  struct Layer {
    int z_index;
    std::vector<Batch> batches;
    Layer(int z_index) : z_index(z_index) {}
  };
  std::map<int, Layer> layers;
};

class DrawBuilder {
  enum class Context { Group, Scene2d, Scene3d };
  enum class DrawType2d { None, Shape2d, Image, Text };

public:
  void begin_group(int z_index, const Box2& mask) {
    next_batch = NextBatch{Context::Group, z_index, mask};
  }

  void queue_shape_2d(const Shape2dInstance& shape) {
    prepare_batch_2d(DrawType2d::Shape2d);
    batch->image_count++;
  }

  void queue_text(
      const FontAtlas& font_atlas,
      const Vec2& origin,
      double angle,
      const Vec2& scale,
      const Color& color,
      Length width,
      const std::string& text) {
    prepare_glyph_group(font_atlas.texture(), color);
    glyph_group->count += font_atlas.add_glyphs(
        dl.glyph_2d_vertices,
        origin,
        angle,
        scale,
        text,
        width);
  }

  void queue_image(unsigned int image_texture, const Mat3& M) {
    prepare_batch_2d(DrawType2d::Image);
    dl.images.emplace_back(DrawList::Image{image_texture, M});
    batch->image_count++;
  }

  DrawList flush() {
    layer = nullptr;
    batch = nullptr;
    prev_type_2d = DrawType2d::None;
    next_batch.reset();
    return std::move(dl);
  }

private:
  void prepare_batch_2d(DrawType2d type_2d) {
    if (next_batch) {
      context = next_batch->context;
      new_batch(next_batch->z_index, next_batch->mask);
      next_batch.reset();
      prev_type_2d = type_2d;
      return;
    }
    if (!batch) {
      throw std::logic_error("Must configure an initial batch");
    }
    if (context != Context::Scene2d) {
      return;
    }
    if (int(type_2d) > int(prev_type_2d)) {
      new_batch(layer->z_index, batch->mask);
    }
    prev_type_2d = type_2d;
  }
  void prepare_glyph_group(unsigned int font_texture, const Color& color) {
    if (!glyph_group || !glyph_group->matches(font_texture, color)) {
      batch->glyph_2d_group_count++;
      glyph_group = &dl.glyph_2d_groups.emplace_back(font_texture, color);
      glyph_group->offset = dl.glyph_2d_vertices.size();
    }
  }

  void new_batch(int z_index, const Box2& mask) {
    if (!layer || layer->z_index != z_index) {
      dl.layers.emplace(z_index, z_index);
    }
    layer = &dl.layers.find(z_index)->second;
    batch = &layer->batches.emplace_back(mask);
    batch->shape_2d_offset = dl.shape_2d_instances.size();
    batch->glyph_2d_group_offset = dl.glyph_2d_groups.size();
    batch->image_offset = dl.images.size();
    prev_type_2d = DrawType2d::None;
  }

  DrawList dl;
  Context context;
  DrawList::Layer* layer = nullptr;
  DrawList::Batch* batch = nullptr;
  DrawList::Glyph2dGroup* glyph_group = nullptr;
  DrawType2d prev_type_2d = DrawType2d::None;

  struct NextBatch {
    Context context;
    int z_index;
    Box2 mask;
  };
  std::optional<NextBatch> next_batch;
};

} // namespace dgui
