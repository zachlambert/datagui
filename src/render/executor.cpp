#include "datagui/render/executor.hpp"
#include "datagui/render/program_registry.hpp"
#include <GL/glew.h>

namespace dgui {

void Executor::draw(
    const Vec2& size,
    ProgramRegistry& registry,
    const DrawList& dl) {

  const Mat3 screen_PV = Mat3{
      {1 / (0.5f * size.x), 0, -1.f},
      {0, -1 / (0.5f * size.y), 1.f},
      {0, 0, 1.f},
  };
  Mat3 PV = screen_PV;

  glViewport(0, 0, (int)size.x, (int)size.y);
  glScissor(0, 0, (int)size.x, (int)size.y);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_SCISSOR_TEST);

  auto set_mask = [&](const Box2& mask) {
    const int scissor_x = std::max<int>(std::floor(mask.lower.x), 0);
    const float scissor_y_float = size.y - mask.upper.y;
    const int scissor_y = std::max<int>(scissor_y_float, 0);
    const int scissor_w = std::min<int>(std::ceil(mask.size_x()), size.x);
    const int scissor_h = std::min<int>(std::ceil(mask.size_y()), size.y);
    glScissor(scissor_x, scissor_y, scissor_w, scissor_h);
  };

  for (const auto& action : dl.actions) {
    if (auto begin_group = std::get_if<DrawList::BeginGroup>(&action)) {
      set_mask(begin_group->mask);
      PV = screen_PV;
    } else if (
        auto begin_scene_2d = std::get_if<DrawList::BeginScene2d>(&action)) {
      const Box2& mask = begin_scene_2d->viewport;
      set_mask(mask);
      const Mat3 viewport_to_screen = Mat3{
          {mask.size_x() / 2, 0.f, mask.center_x()},
          {0.f, -mask.size_y() / 2, mask.center_y()},
          {0.f, 0.f, 1.f}};
      const auto& camera = begin_scene_2d->camera;
      const Mat3 camera_PV = camera.projection_mat() * camera.view_mat();
      PV = screen_PV * viewport_to_screen * camera_PV;
    } else if (
        auto draw_shape_2d = std::get_if<DrawList::DrawShape2d>(&action)) {
      registry.shape_2d_program.bind();
      registry.shape_2d_program.draw(
          PV,
          &dl.shape_2d_instances[draw_shape_2d->offset],
          draw_shape_2d->count);
    } else if (
        auto draw_glyph_2d = std::get_if<DrawList::DrawGlyph2d>(&action)) {
      registry.glyph_2d_program.bind();
      registry.glyph_2d_program.draw(
          PV,
          draw_glyph_2d->font_texture,
          &dl.glyph_2d_instances[draw_glyph_2d->offset],
          draw_glyph_2d->count);
    } else if (
        auto draw_image_2d = std::get_if<DrawList::DrawImage2d>(&action)) {
      registry.image_2d_program.bind();
      registry.image_2d_program.draw(
          PV,
          draw_image_2d->image_texture,
          draw_image_2d->transform);
    }
  }

  // If kept enabled, then glClear will only clear within the scissor region
  glDisable(GL_SCISSOR_TEST);
}

} // namespace dgui
