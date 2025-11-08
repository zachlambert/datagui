#include "datagui/element/floating.hpp"

namespace datagui {

void FloatingSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  auto& props = *e.props.cast<FloatingProps>();
  e.fixed_size = Vecf::Zero();
  e.dynamic_size = Vecf::Zero();
  e.floating = true;
  e.floating_type = FloatingTypeAbsolute(Vecf(props.width, props.height));
}

void FloatingSystem::set_dependent_state(
    Element& e,
    const ElementList& children) {

  auto& props = *e.props.cast<FloatingProps>();

  {
    auto& box = props.title_bar_box;
    box.lower = e.float_box.lower;
    box.upper.x = e.float_box.upper.x;
    box.upper.y = e.float_box.lower.y +
                  fm->text_height(theme->text_font, theme->text_size) +
                  2.f * (theme->input_border_width + theme->text_padding);
  }

  props.title_bar_text_width =
      e.float_box.size().x -
      2.f * (theme->input_border_width + theme->text_padding);

  {
    auto& box = props.close_button_box;
    Vecf text_size = fm->text_size(
        "close",
        theme->text_font,
        theme->text_size,
        LengthWrap());

    Vecf button_size = text_size + 2.f * Vecf::Constant(theme->text_padding);
    props.title_bar_text_width -= (button_size.x + theme->text_padding);
    props.title_bar_text_width = std::max(0.f, props.title_bar_text_width);

    box.upper =
        props.title_bar_box.upper - Vecf::Constant(theme->input_border_width);
    box.lower = box.upper - button_size;
  }

  if (children.empty()) {
    return;
  }

  // Always hide excess children
  for (std::size_t i = 1; i < children.size(); i++) {
    children[i]->hidden = true;
  }

  auto& child = *children[0];

  child.position = props.title_bar_box.top_left();
  child.size = e.float_box.upper - child.position;
}

void FloatingSystem::render(const Element& e, Renderer& renderer) {
  auto& props = *e.props.cast<FloatingProps>();

  const Color& bg_color =
      props.bg_color ? *props.bg_color : theme->layout_color_bg;

  renderer.queue_box(
      e.float_box,
      bg_color,
      theme->popup_border_width,
      Color::Black(),
      0);

  renderer.queue_box(
      props.title_bar_box,
      theme->layout_color_bg,
      theme->input_border_width,
      theme->input_color_border,
      0);

  renderer.queue_text(
      e.float_box.lower +
          Vecf::Constant(theme->input_border_width + theme->text_padding),
      props.title,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthFixed(props.title_bar_text_width));

  renderer.queue_box(
      props.close_button_box,
      theme->input_color_bg,
      0,
      Color::Black(),
      0);

  renderer.queue_text(
      props.close_button_box.lower + Vecf::Constant(theme->text_padding),
      "close",
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
}

bool FloatingSystem::mouse_event(Element& e, const MouseEvent& event) {
  if (event.action != MouseAction::Release) {
    return false;
  }
  auto& props = *e.props.cast<FloatingProps>();

  if (!props.close_button_box.contains(event.position)) {
    return false;
  }
  props.open = false;
  props.open_changed = true;
  return true;
}

} // namespace datagui
