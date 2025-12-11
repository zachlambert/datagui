#pragma once

#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/image_shader.hpp"
#include "datagui/visual/shape_shader.hpp"
#include "datagui/visual/text_shader.hpp"
#include <map>

namespace datagui {

class Canvas2 : public Viewport {
public:
  void box(
      const Box2& box,
      const Color& color,
      float radius = 0,
      float border_width = 0,
      const Color& border_color = Color::Black(),
      int z_index = 0);

  void text(
      const Box2& box,
      const Color& color,
      float radius = 0,
      float border_width = 0,
      const Color& border_color = Color::Black(),
      int z_index = 0);

private:
  void begin() override;
  void end() override;
  void mouse_event(const Box2& box, const MouseEvent& event) override;

  void impl_init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm) override;
  void impl_render() override;

  struct Layer {
    ShapeShader::Command shape_command;
    TextShader::Command text_command;
  };
  std::map<int, Layer> layers;
  Layer& get_layer(int z_index);

  ShapeShader shape_shader;
  TextShader text_shader;
};

} // namespace datagui
