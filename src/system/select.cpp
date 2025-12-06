#include "datagui/system/select.hpp"

namespace datagui {

void SelectSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& select = element.select();

  const Vec2 none_size =
      fm->text_size("<none>", theme->text_font, theme->text_size, LengthWrap());

  float max_item_width = none_size.x;
  for (const auto& choice : select.choices) {
    Vec2 choice_size =
        fm->text_size(choice, theme->text_font, theme->text_size, LengthWrap());
    max_item_width = std::max(max_item_width, choice_size.x);
  }

  state.fixed_size = Vec2();
  state.dynamic_size = Vec2();

  if (auto width = std::get_if<LengthFixed>(&select.width)) {
    state.fixed_size.x = std::min(max_item_width, width->value);
  } else if (std::get_if<LengthWrap>(&select.width)) {
    state.fixed_size.x = max_item_width;
  } else if (auto width = std::get_if<LengthDynamic>(&select.width)) {
    state.dynamic_size.x = width->weight;
  }
  state.fixed_size +=
      2.f * Vec2::uniform(theme->input_border_width + theme->text_padding);
  state.fixed_size.y += fm->text_height(theme->text_font, theme->text_size);

  if (!select.choices.empty()) {
    state.floating = select.open;

    Vec2 select_size;
    select_size.x = state.fixed_size.x;
    select_size.y = select.choices.size() *
                        fm->text_height(theme->text_font, theme->text_size) +
                    (select.choices.size() + 1) *
                        (theme->input_border_width + theme->text_padding);
    state.floating_type = FloatingTypeRelative(Vec2(), select_size);

  } else {
    state.floating = false;
  }
}

void SelectSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  const auto& select = element.select();

  state.float_box = state.box();
  if (!state.floating) {
    return;
  }

  state.float_box.upper.y +=
      (select.choices.size() - 1) *
      (fm->text_height(theme->text_font, theme->text_size) +
       2.f * theme->text_padding + theme->input_border_width);
}

void SelectSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& select = element.select();

  if (!select.open || select.choices.empty()) {
    renderer.queue_box(
        state.box(),
        theme->input_color_bg,
        theme->input_border_width,
        theme->input_color_border);

    std::string text;
    if (select.choices.empty() || select.choice == -1) {
      text = "<none>";
    } else {
      text = select.choices[select.choice];
    }

    renderer.queue_text(
        state.position +
            Vec2::uniform(theme->input_border_width + theme->text_padding),
        text,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthFixed(
            state.size.x -
            2.f * (theme->input_border_width + theme->text_padding)));
    return;
  }

  Vec2 offset;

  for (int i = 0; i < select.choices.size(); i++) {
    Vec2 position = state.position + offset;
    Vec2 size = Vec2(
        state.size.x,
        fm->text_height(theme->text_font, theme->text_size) +
            2.f * (theme->text_padding + theme->input_border_width));

    Color bg_color;
    if (i == select.choice) {
      bg_color = theme->input_color_bg_active;
    } else {
      bg_color = theme->input_color_bg;
    }
    renderer.queue_box(
        Box2(position, position + size),
        bg_color,
        theme->input_border_width,
        theme->input_color_border);

    renderer.queue_text(
        position +
            Vec2::uniform(theme->input_border_width + theme->text_padding),
        select.choices[i],
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthFixed(
            state.size.x -
            2.f * (theme->input_border_width + theme->text_padding)));

    offset.y += theme->input_border_width + 2 * theme->text_padding +
                fm->text_height(theme->text_font, theme->text_size);
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

  Vec2 offset;
  int clicked = -1;

  for (int i = 0; i < select.choices.size(); i++) {
    Vec2 position = state.position + offset;
    Vec2 size = Vec2(
        state.size.x,
        fm->text_height(theme->text_font, theme->text_size) +
            2.f * (theme->text_padding + theme->input_border_width));

    if (Box2(position, position + size).contains(event.position)) {
      clicked = i;
      break;
    }

    offset.y += theme->input_border_width + 2.f * theme->text_padding +
                fm->text_height(theme->text_font, theme->text_size);
  }

  if (clicked != -1) {
    select.open = false;
    if (select.choice != clicked) {
      select.choice = clicked;
      if (select.callback) {
        select.callback(select.choice);
      } else {
        element.set_dirty();
      }
    }
  }
}

void SelectSystem::focus_enter(ElementPtr element) {
  auto& select = element.select();
  select.open = true;
}

bool SelectSystem::focus_leave(ElementPtr element, bool success) {
  auto& select = element.select();
  select.open = false;
  return false;
}

} // namespace datagui
