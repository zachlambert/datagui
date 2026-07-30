#include "datagui/system/tabs.hpp"

namespace dgui {

void TabsSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& tabs = element.tabs();

  state.fixed_size = Vec2();
  state.dynamic_size = Vec2();
  auto child = element.child();
  for (std::size_t i = 0; i < tabs.labels.size(); i++) {
    if (child.state().float_only) {
      child = child.next();
      continue;
    }
    if (!child) {
      break;
    }
    state.fixed_size = maximum(state.fixed_size, child.state().fixed_size);
    state.dynamic_size =
        maximum(state.dynamic_size, child.state().dynamic_size);
    child = child.next();
  }

  const auto& font =
      font_registry->get_font(theme->text_font, theme->text_size);

  tabs.header_height = font.text_height() + 2 * theme->text_padding +
                       2 * theme->input_border_width;
  state.fixed_size.y += tabs.header_height;

  tabs.label_boxes.resize(tabs.labels.size());
  Vec2 offset;
  for (std::size_t i = 0; i < tabs.labels.size(); i++) {
    const auto& label = tabs.labels[i];
    Vec2 label_size =
        font.text_size(label, LengthWrap()) +
        Vec2::uniform(2 * theme->text_padding + 2 * theme->input_border_width);

    tabs.label_boxes[i] = Box2(offset, offset + label_size);
    offset.x += label_size.x - theme->input_border_width;
  }
  offset.x += theme->input_border_width;
  state.fixed_size.x = std::max(offset.x, state.fixed_size.x);
}

void TabsSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& tabs = element.tabs();

  for (auto& box : tabs.label_boxes) {
    box.lower += state.position;
    box.upper += state.position;
  }

  Box2 content_box = state.box();
  content_box.lower.y += tabs.header_height;

  Vec2 child_pos = content_box.lower;
  Vec2 child_full_size = content_box.size();

  auto child = element.child();
  std::size_t i = 0;
  while (child) {
    if (child.state().float_only) {
      child = child.next();
      continue;
    }
    child.state().position = child_pos;
    auto& c_state = child.state();
    if (c_state.dynamic_size.x > 0) {
      c_state.size.x = child_full_size.x;
    } else {
      c_state.size.x = c_state.fixed_size.x;
    }
    if (c_state.dynamic_size.y > 0) {
      c_state.size.y = child_full_size.y;
    } else {
      c_state.size.y = c_state.fixed_size.y;
    }
    assert(child.state().hidden == (i != tabs.tab));
    i++;
    child = child.next();
  }
}

void TabsSystem::render(ConstElementPtr element, DrawList& dl) {
  const auto& state = element.state();
  const auto& tabs = element.tabs();

  const auto& font =
      font_registry->get_font(theme->text_font, theme->text_size);

  Box2 header_box;
  header_box.lower = state.position;
  header_box.upper = state.position + Vec2(state.size.x, tabs.header_height);
  dl.draw_box(
      header_box,
      theme->header_color,
      theme->input_border_width,
      theme->layout_border_color);

  Vec2 text_offset =
      Vec2::uniform(theme->input_border_width + theme->text_padding);
  for (std::size_t i = 0; i < tabs.labels.size(); i++) {
    const Color& bg_color =
        (i == tabs.tab) ? theme->input_color_bg_active : theme->input_color_bg;
    const auto& box = tabs.label_boxes[i];
    dl.draw_box(
        box,
        bg_color,
        theme->input_border_width,
        theme->input_color_border);
    dl.draw_text(
        font,
        box.lower + text_offset,
        theme->text_color,
        LengthWrap(),
        tabs.labels[i]);
  }
}

void TabsSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& tabs = element.tabs();

  if (event.action != MouseAction::Press) {
    return;
  }

  for (std::size_t i = 0; i < tabs.labels.size(); i++) {
    if (tabs.label_boxes[i].contains(event.position)) {
      if (i != tabs.tab) {
        tabs.tab = i;
      }
      return;
    }
  }
}

} // namespace dgui
