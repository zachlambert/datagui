#pragma once

#include "datagui/visual/font_manager.hpp"
#include "datagui/visual/image_shader.hpp"
#include "datagui/visual/shape_shader.hpp"
#include "datagui/visual/text_shader.hpp"
#include <assert.h>
#include <memory>
#include <stack>

namespace datagui {

class Renderer {
public:
  void init(std::shared_ptr<FontManager> fm);

  void queue_box(
      const Box2& box,
      const Color& bg_color,
      float border_width = 0,
      Color border_color = Color::Black());

  void queue_text(
      const Vec2& origin,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width = LengthWrap());

  void queue_image(const Box2& box, unsigned int texture);
  void queue_image(
      const Box2& box,
      std::size_t width,
      std::size_t height,
      void* pixels);

  void render_begin(const Vec2& viewport_size);
  void render_end();

  void new_layer();
  void push_mask(const Box2& mask);
  void pop_mask();

private:
  Box2 flip_box(const Box2& box);
  Vec2 flip_position(const Vec2& origin, Font font, int font_size);

  struct ImageCommand {
    Box2 box;
    // Use either the texture or image
    unsigned int texture;
    OpenglRgbImage image;
  };
  struct Layer {
    ShapeShader::Command shape_command;
    TextShader::Command text_command;
    std::vector<ImageCommand> image_commands;
  };
  std::vector<Layer> layers;

  ShapeShader shape_shader;
  TextShader text_shader;
  ImageShader image_shader;

  std::shared_ptr<FontManager> fm;
  Vec2 viewport_size;
  std::stack<Box2> masks;
};

} // namespace datagui
