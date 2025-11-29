#include "datagui/system/dropdown.hpp"

namespace datagui {

void DropdownSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& dropdown = element.dropdown();

  const Vec2 none_size =
      fm->text_size("<none>", theme->text_font, theme->text_size, LengthWrap());

  float max_item_width = none_size.x;
  for (const auto& choice : dropdown.choices) {
    Vec2 choice_size =
        fm->text_size(choice, theme->text_font, theme->text_size, LengthWrap());
    max_item_width = std::max(max_item_width, choice_size.x);
  }

  state.fixed_size = Vec2();
  state.dynamic_size = Vec2();

  if (auto width = std::get_if<LengthFixed>(&dropdown.width)) {
    state.fixed_size.x = std::min(max_item_width, width->value);
  } else if (std::get_if<LengthWrap>(&dropdown.width)) {
    state.fixed_size.x = max_item_width;
  } else if (auto width = std::get_if<LengthDynamic>(&dropdown.width)) {
    state.dynamic_size.x = width->weight;
  }
  state.fixed_size +=
      2.f * Vec2::uniform(theme->input_border_width + theme->text_padding);
  state.fixed_size.y += fm->text_height(theme->text_font, theme->text_size);

  if (!dropdown.choices.empty()) {
    state.floating = dropdown.open;

    Vec2 dropdown_size;
    dropdown_size.x = state.fixed_size.x;
    dropdown_size.y = dropdown.choices.size() *
                          fm->text_height(theme->text_font, theme->text_size) +
                      (dropdown.choices.size() + 1) *
                          (theme->input_border_width + theme->text_padding);
    state.floating_type = FloatingTypeRelative(Vec2(), dropdown_size);

  } else {
    state.floating = false;
  }
}

void DropdownSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  const auto& dropdown = element.dropdown();

  state.float_box = state.box();
  if (!state.floating) {
    return;
  }

  state.float_box.upper.y +=
      (dropdown.choices.size() - 1) *
      (fm->text_height(theme->text_font, theme->text_size) +
       2.f * theme->text_padding + theme->input_border_width);
}

void DropdownSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& dropdown = element.dropdown();

  if (!dropdown.open || dropdown.choices.empty()) {
    renderer.queue_box(
        state.box(),
        theme->input_color_bg,
        theme->input_border_width,
        theme->input_color_border,
        0);

    std::string text;
    if (dropdown.choices.empty() || dropdown.choice == -1) {
      text = "<none>";
    } else {
      text = dropdown.choices[dropdown.choice];
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

  for (int i = 0; i < dropdown.choices.size(); i++) {
    Vec2 position = state.position + offset;
    Vec2 size = Vec2(
        state.size.x,
        fm->text_height(theme->text_font, theme->text_size) +
            2.f * (theme->text_padding + theme->input_border_width));

    Color bg_color;
    if (i == dropdown.choice) {
      bg_color = theme->input_color_bg_active;
    } else {
      bg_color = theme->input_color_bg;
    }
    renderer.queue_box(
        Box2(position, position + size),
        bg_color,
        theme->input_border_width,
        theme->input_color_border,
        0);

    renderer.queue_text(
        position +
            Vec2::uniform(theme->input_border_width + theme->text_padding),
        dropdown.choices[i],
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

bool DropdownSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& dropdown = element.dropdown();

  if (event.action != MouseAction::Press) {
    return false;
  }

  if (!dropdown.open) {
    dropdown.open = true;
    return false;
  }
  if (dropdown.choices.empty()) {
    return false;
  }

  Vec2 offset;
  int clicked = -1;

  for (int i = 0; i < dropdown.choices.size(); i++) {
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
    dropdown.open = false;
    if (dropdown.choice != clicked) {
      dropdown.choice = clicked;
      if (!dropdown.callback) {
        return true;
      }
      dropdown.callback(dropdown.choice);
    }
  }
  return false;
}

void DropdownSystem::focus_enter(ElementPtr element) {
  auto& dropdown = element.dropdown();
  dropdown.open = true;
}

bool DropdownSystem::focus_leave(ElementPtr element, bool success) {
  auto& dropdown = element.dropdown();
  dropdown.open = false;
  return false;
}

} // namespace datagui
