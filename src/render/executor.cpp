#include "datagui/render/executor.hpp"
#include "datagui/render/program_registry.hpp"
#include <GL/glew.h>

namespace dgui {

void Executor::draw(
    const Vec2& size,
    ProgramRegistry& registry,
    const DrawList& dl) {

  // clang-format off
  const Mat3 screen_PV = Mat3{
      1 / (0.5f * size.x), 0, -1,
      0, -1 / (0.5f * size.y), 1,
      0, 0, 1,
  };
  const Mat4 screen_PV_3d = Mat4{
      1 / (0.5f * size.x), 0, 0, -1,
      0, -1 / (0.5f * size.y), 0, 1,
      0, 0, 1, 0,
      0, 0, 0, 1,
  };
  // clang-format on

  auto set_viewport = [&](const Box2& mask) {
    const int x = std::floor(mask.lower.x);
    const float y_float = size.y - mask.upper.y;
    const int y = std::floor(size.y - mask.upper.y);
    const int w = std::ceil(mask.size_x());
    const int h = std::ceil(mask.size_y());
    glViewport(x, y, w, h);
  };
  auto reset_viewport = [&]() {
    glViewport(0, 0, (int)size.x, (int)size.y);
  };

  auto set_mask = [&](const Box2& mask) {
    const int x = std::max<int>(std::floor(mask.lower.x), 0);
    const int y = std::max<int>(std::floor(size.y - mask.upper.y), 0);
    const int w = std::min<int>(std::ceil(mask.size_x()), size.x);
    const int h = std::min<int>(std::ceil(mask.size_y()), size.y);
    glScissor(x, y, w, h);
  };
  auto clear_mask = [&]() {
    glScissor(0, 0, (int)size.x, (int)size.y);
  };

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_SCISSOR_TEST);

  for (const auto& group : dl.groups) {
    reset_viewport();
    set_mask(group.mask);

    if (group.shape_count > 0) {
      registry.shape_2d_program.bind();
      registry.shape_2d_program.draw(
          screen_PV,
          &dl.shape_instances[group.shape_offset],
          group.shape_count);
    }

    if (group.glyph_group_count > 0) {
      registry.glyph_2d_program.bind();
      for (size_t i = 0; i < group.glyph_group_count; i++) {
        const auto& glyphs = dl.glyph_groups[group.glyph_group_offset + i];
        registry.glyph_2d_program.draw(
            screen_PV,
            glyphs.font_texture,
            &dl.glyph_instances[glyphs.offset],
            glyphs.count);
      }
    }

    if (group.image_count > 0) {
      registry.image_2d_program.bind();
      for (size_t i = 0; i < group.image_count; i++) {
        const auto& instance = dl.image_instances[group.image_offset + i];
        registry.image_2d_program.draw(
            screen_PV,
            instance.image.texture(),
            instance.transform);
      }
    }

    // Draw scene backgrounds first to avoid changing the viewport
    if (group.scene_2d_count > 0 || group.scene_3d_count > 0) {
      registry.shape_2d_program.bind();
      for (size_t i = 0; i < group.scene_2d_count; i++) {
        const auto& instance = dl.scene_2d_instances[group.scene_2d_offset + i];
        auto shape = Shape2dInstance::box(
            instance.viewport,
            instance.scene_2d->bg_color,
            0,
            Color::Black(),
            0);
        registry.shape_2d_program.draw(screen_PV, &shape, 1);
      }
      for (size_t i = 0; i < group.scene_3d_count; i++) {
        const auto& instance = dl.scene_3d_instances[group.scene_3d_offset + i];
        auto shape = Shape2dInstance::box(
            instance.viewport,
            instance.scene_3d->bg_color,
            0,
            Color::Black(),
            0);
        registry.shape_2d_program.draw(screen_PV, &shape, 1);
      }
    }

    for (size_t i = 0; i < group.scene_2d_count; i++) {
      const auto& instance = dl.scene_2d_instances[group.scene_2d_offset + i];
      const auto& [viewport, camera, scene] = instance;

      set_mask(intersection(viewport, group.mask));
      set_viewport(viewport);
      const Mat3 PV = camera.projection_mat() * camera.view_mat();

      for (const auto& action : scene->actions) {
        if (auto draw_shape = std::get_if<Scene2d::DrawShape>(&action)) {
          registry.shape_2d_program.bind();
          registry.shape_2d_program.draw(
              PV,
              &scene->shape_instances[draw_shape->offset],
              draw_shape->count);
        } else if (auto draw_glyph = std::get_if<Scene2d::DrawGlyph>(&action)) {
          registry.glyph_2d_program.bind();
          registry.glyph_2d_program.draw(
              PV,
              draw_glyph->font_texture,
              &scene->glyph_instances[draw_glyph->offset],
              draw_glyph->count);
        } else if (auto draw_image = std::get_if<Scene2d::DrawImage>(&action)) {
          registry.image_2d_program.bind();
          registry.image_2d_program.draw(
              PV,
              draw_image->image.texture(),
              draw_image->transform);
        }
      }
    }

    for (size_t i = 0; i < group.scene_3d_count; i++) {
      const auto& instance = dl.scene_3d_instances[group.scene_3d_offset + i];
      const auto& [viewport, camera, scene] = instance;

      set_mask(intersection(viewport, group.mask));
      set_viewport(viewport);
      const Mat4 projection = camera.projection_mat();
      const Mat4 view = camera.view_mat();

      glDisable(GL_BLEND);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);
      glClear(GL_DEPTH_BUFFER_BIT);

      registry.shape_3d_program.bind();
      for (size_t i = 0; i < size_t(Shape3dTypeCount); i++) {
        const auto& shapes = scene->shape_instances[i];
        registry.shape_3d_program.draw(
            view,
            projection,
            Shape3dType(i),
            shapes.data(),
            shapes.size());
      }
      if (!scene->mesh_instances.empty()) {
        registry.mesh_program.bind();
        for (const auto& instance : scene->mesh_instances) {
          registry.mesh_program.draw(
              view,
              projection,
              instance.mesh,
              instance.transform,
              instance.base_color);
        }
      }
      if (!scene->point_cloud_instances.empty()) {
        registry.point_cloud_program.bind();
        for (const auto& instance : scene->point_cloud_instances) {
          registry.point_cloud_program.draw(
              view,
              projection,
              instance.point_cloud,
              instance.transform,
              instance.point_size,
              instance.base_color);
        }
      }

      glEnable(GL_BLEND);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
    }
  }

  // If kept enabled, then glClear will only clear within the scissor region
  glDisable(GL_SCISSOR_TEST);
}

} // namespace dgui
