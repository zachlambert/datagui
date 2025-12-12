#pragma once

#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/shape_3d_shader.hpp"

namespace datagui {

class Canvas3d : public Viewport {
public:
  Canvas3d();

  void box(
      const Vec3& position,
      const Rot3& orientation,
      const Vec3& size,
      const Color& color);

  void grid(std::size_t size, float width);

private:
  void begin() override;
  void end() override;
  void impl_init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm) override;
  void mouse_event(const Vec2& size, const MouseEvent& event) override;

  Camera3d camera;
  Shape3dShader shape_shader;
};

} // namespace datagui
