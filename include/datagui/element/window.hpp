#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"
#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"
#include <optional>
#include <variant>

namespace datagui {

struct WindowPositionAbsolute {
  BoxDims margin;
};

struct WindowPositionRelative {
  Vecf offset;
  Vecf size;
};

using WindowPosition =
    std::variant<WindowPositionAbsolute, WindowPositionRelative>;

struct TitleBarStyle : public TextStyle {
  Color bg_color = Color::Gray(0.8);
  bool close_button = true;
  Color close_button_color = Color::Gray(0.6);
  BoxDims padding = 5;
  BoxDims border_width = 0;
  Color border_color = Color::Black();
};

struct WindowStyle {
  WindowPosition position;
  Color bg_color = Color::White();
  std::optional<TitleBarStyle> title_bar;
};
using SetWindowStyle = SetStyle<WindowStyle>;

struct WindowData {
  WindowStyle style;
  std::string title;
  bool open = false;
  bool open_changed = false;
};

class WindowSystem : public ElementSystemImpl<WindowData> {
public:
  WindowSystem(
      GeometryRenderer& geometry_renderer,
      TextRenderer& text_renderer,
      FontManager& font_manager) :
      geometry_renderer(geometry_renderer),
      text_renderer(text_renderer),
      font_manager(font_manager) {}

  void visit(
      Element element,
      Variable<bool> open,
      const std::string& title,
      const SetWindowStyle& set_style);

  void set_layout_input(Element element) const override;
  void set_child_layout_output(Element elment) const override;
  void set_float_box(ConstElement window, Element element) const override;
  void render(ConstElement element) const override;
  void mouse_event(Element element, const MouseEvent& event) override;

private:
  GeometryRenderer& geometry_renderer;
  TextRenderer& text_renderer;
  FontManager& font_manager;
};

} // namespace datagui
