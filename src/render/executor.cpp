#include "datagui/render/executor.hpp"
#include "datagui/render/program_registry.hpp"
#include <GL/glew.h>

namespace dgui {

void Executor::draw(
    const Vec2& size,
    ProgramRegistry& registry,
    const DrawList& dl) {

  const Mat3 PV = Mat3{
      {1 / (0.5f * size.x), 0, -1.f},
      {0, 1 / (0.5f * size.y), -1.f},
      {0, 0, 1.f},
  };

  for (const auto& [_, batches] : dl.batches) {
    for (const auto& batch : batches) {
      const int x = std::floor(batch.mask.lower.x);
      const int y = std::floor(batch.mask.lower.y);
      const int w = std::ceil(batch.mask.upper.x - batch.mask.lower.x);
      const int h = std::ceil(batch.mask.upper.y - batch.mask.lower.y);
      glScissor(x, y, w, h);

      if (batch.shape_count > 0) {
        registry.shape_2d_program.bind();
        registry.shape_2d_program.draw(
            &dl.shape_2d_instances[batch.shape_offset],
            batch.shape_count,
            PV);
      }
      if (!batch.image_groups.empty()) {
        registry.image_2d_program.bind();
        for (const auto& group : batch.image_groups) {
          registry.image_2d_program.draw(
              group.image.texture(),
              &dl.image_2d_vertices[group.offset],
              group.count,
              PV);
        }
      }
      if (!batch.glyph_groups.empty()) {
        registry.glyph_2d_program.bind();
        for (const auto& group : batch.glyph_groups) {
          registry.glyph_2d_program.draw(
              group.font_texture,
              group.color,
              &dl.glyph_2d_vertices[group.offset],
              group.count,
              PV);
        }
      }
    }
  }
}

} // namespace dgui
