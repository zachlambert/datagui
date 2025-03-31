#pragma once

#include "datagui/color.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font.hpp"
#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

struct TextInputStyle {
  float max_width;
  Font font = Font::DejaVuSans;
  int font_size = 24;
  Color text_color;
  Color bg_color;
  float padding;
  float border_width;
  Color border_color;
  Color focus_color;
};

struct TextInputElement {
  using Style = TextInputStyle;
  Style style;
};

class TextInputSystem : public ElementSystemBase<TextInputElement> {
public:
  TextInputSystem(
      FontManager& font_manager,
      TextRenderer& text_renderer,
      GeometryRenderer& geometry_renderer) :
      font_manager(font_manager),
      text_renderer(text_renderer),
      geometry_renderer(geometry_renderer) {}

  void init(const std::function<void(TextInputStyle&)> set_style = nullptr);
  void set_layout_input(Tree::Ptr node) const override;
  void render(Tree::ConstPtr node) const override;

#if 0

  bool press(const Node& node, const Vecf& mouse_pos) override;
  bool held(const Node& node, const Vecf& mouse_pos) override;
  bool focus_enter(const Node& node) override;
  bool focus_leave(
      const Tree& tree,
      const Node& node,
      bool success,
      int new_focus) override;
  bool key_event(const Node& node, const KeyEvent& event) override;

#endif

private:
  FontManager& font_manager;
  TextRenderer& text_renderer;
  GeometryRenderer& geometry_renderer;
};

} // namespace datagui
