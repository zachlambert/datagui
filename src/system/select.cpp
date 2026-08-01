#include "datagui/system/select.hpp"

namespace dgui {

void SelectSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& select = element.select();

  const auto& font =
      font_registry->get_font(theme->text_font, theme->text_size);

  float text_height = font.text_height();
  select.max_item_width = theme->select_min_width;
  for (const auto& choice : select.choices) {
    Vec2 choice_size = font.text_size(choice, LengthWrap());
    select.max_item_width = std::max(select.max_item_width, choice_size.x);
  }

  float padding = theme->text_padding + theme->input_border_width;
  state.fixed_size.x = select.max_item_width + 2 * padding;
  state.fixed_size.y = text_height + 2 * padding;
  state.dynamic_size = Vec2();

  if (!select.choices.empty()) {
    state.content_visible = select.open;
  } else {
    state.content_visible = false;
  }
  state.content_floating = true;
}

void SelectSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& select = element.select();

  const auto& font =
      font_registry->get_font(theme->text_font, theme->text_size);
  float text_height = font.text_height();

  Vec2 float_size;
  float_size.x = state.size.x;
  float_size.y =
      select.choices.size() * (text_height + 2 * theme->text_padding) +
      (select.choices.size() + 1) * theme->input_border_width;

  Vec2 float_offset;
  float_offset.x = 0;
  float_offset.y = state.fixed_size.y - theme->input_border_width;

  state.content_box =
      Box2::from_size(state.position + float_offset, float_size);

  select.choice_boxes.resize(select.choices.size());

  Vec2 item_position = state.content_box.lower;
  Vec2 item_size = state.box().size();
  for (std::size_t i = 0; i < select.choices.size(); i++) {
    auto& box = select.choice_boxes[i];
    box = Box2::from_size(item_position, item_size);
    item_position.y += (item_size.y - theme->input_border_width);
  }
}

void SelectSystem::render(ConstElementPtr element, DrawList& dl) {
  const auto& state = element.state();
  const auto& select = element.select();

  const auto& font =
      font_registry->get_font(theme->text_font, theme->text_size);

  const Color& select_color =
      select.open ? theme->input_color_bg_active : theme->input_color_bg;
  dl.draw_box(
      state.box(),
      select_color,
      theme->input_border_width,
      theme->input_color_border);

  if (!select.choices.empty() && select.choice >= 0) {
    Vec2 offset =
        Vec2::uniform(theme->text_padding + theme->input_border_width);
    dl.draw_text(
        font,
        state.position + offset,
        theme->text_color,
        LengthWrap(),
        select.choices[select.choice]);
  }
}

void SelectSystem::render_content(ConstElementPtr element, DrawList& dl) {
  const auto& state = element.state();
  const auto& select = element.select();
  const auto& font =
      font_registry->get_font(theme->text_font, theme->text_size);

  for (std::size_t i = 0; i < select.choices.size(); i++) {
    const auto& box = select.choice_boxes[i];
    const Color& bg_color = (i == select.choice) ? theme->input_color_bg_active
                                                 : theme->input_color_bg;
    dl.draw_box(
        box,
        bg_color,
        theme->input_border_width,
        theme->input_color_border);

    dl.draw_text(
        font,
        box.lower +
            Vec2::uniform(theme->input_border_width + theme->text_padding),
        theme->text_color,
        LengthFixed(
            state.size.x -
            2.f * (theme->input_border_width + theme->text_padding)),
        select.choices[i]);
  }
}

void SelectSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& select = element.select();

  if (event.action != MouseAction::Press) {
    return;
  }

  if (!select.open) {
    select.open = true;
    return;
  }
  if (select.choices.empty()) {
    return;
  }

  for (std::size_t i = 0; i < select.choices.size(); i++) {
    if (select.choice_boxes[i].contains(event.position)) {
      if (i != select.choice) {
        select.choice = i;
        select.changed = true;
      }
      return;
    }
  }
}

void SelectSystem::focus_enter(ElementPtr element) {
  auto& select = element.select();
  select.open = true;
}

void SelectSystem::focus_leave(ElementPtr element, bool success) {
  auto& select = element.select();
  select.open = false;
}

} // namespace dgui
