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
      {0, -1 / (0.5f * size.y), 1.f},
      {0, 0, 1.f},
  };

  glViewport(0, 0, (int)size.x, (int)size.y);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_SCISSOR_TEST);

  for (const auto& [_, batches] : dl.batches) {
    for (const auto& batch : batches) {
      const int scissor_x = std::max<int>(std::floor(batch.mask.lower.x), 0);
      const int scissor_y = std::max<int>(std::floor(batch.mask.lower.y), 0);
      const int scissor_w = std::min<int>(
          std::ceil(batch.mask.upper.x - batch.mask.lower.x),
          size.x);
      const int scissor_h = std::min<int>(
          std::ceil(batch.mask.upper.y - batch.mask.lower.y),
          size.y);

      glScissor(
          scissor_x,
          size.y - scissor_y - scissor_h,
          scissor_w,
          scissor_h);

      if (batch.shape_count > 0) {
        registry.shape_2d_program.bind();
        registry.shape_2d_program.draw(
            PV,
            &dl.shape_2d_instances[batch.shape_offset],
            batch.shape_count);
      }
      if (!batch.image_groups.empty()) {
        registry.image_2d_program.bind();
#if 0
        for (const auto& group : batch.image_groups) {
          registry.image_2d_program.draw(
            PV,
              group.image.texture(),
              &dl.image_2d_vertices[group.offset],
              group.count);
        }
#endif
      }
      if (!batch.glyph_groups.empty()) {
        registry.glyph_2d_program.bind();
        for (const auto& group : batch.glyph_groups) {
          registry.glyph_2d_program.draw(
              PV,
              group.font_texture,
              &dl.glyph_2d_instances[group.offset],
              group.count);
        }
      }
    }
  }

  // If kept enabled, then glClear will only clear within the scissor region
  glDisable(GL_SCISSOR_TEST);
}

} // namespace dgui
