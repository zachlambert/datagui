#include "datagui/element/dropdown.hpp"

namespace datagui {

void DropdownSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  const auto& props = *e.props.cast<DropdownProps>();

  float max_item_width = 0;
  for (const auto& choice : props.choices) {
    Vecf choice_size =
        fm->text_size(choice, theme->text_font, theme->text_size, LengthWrap());
    max_item_width = std::max(max_item_width, choice_size.x);
  }
  if (props.choices.empty()) {
    Vecf none_size = fm->text_size(
        "<none>",
        theme->text_font,
        theme->text_size,
        LengthWrap());
    max_item_width = std::max(max_item_width, none_size.x);
  }

  e.fixed_size = Vecf::Zero();
  e.dynamic_size = Vecf::Zero();

  if (auto width = std::get_if<LengthFixed>(&props.width)) {
    e.fixed_size.x = std::min(max_item_width, width->value);
  } else if (std::get_if<LengthWrap>(&props.width)) {
    e.fixed_size.x = max_item_width;
  } else if (auto width = std::get_if<LengthDynamic>(&props.width)) {
    e.dynamic_size.x = width->weight;
  }
  e.fixed_size +=
      2.f * Vecf::Constant(theme->input_border_width + theme->text_padding);
  e.fixed_size.y += fm->text_height(theme->text_font, theme->text_size);

  e.floating = props.open && !props.choices.empty();
}

void DropdownSystem::set_dependent_state(
    Element& e,
    const ElementList& children) {
  e.float_box = e.box();
  if (!e.floating) {
    return;
  }

  const auto& props = *e.props.cast<DropdownProps>();
  e.float_box.upper.y +=
      (props.choices.size() - 1) *
      (fm->text_height(theme->text_font, theme->text_size) +
       2.f * theme->text_padding + theme->input_border_width);
}

void DropdownSystem::render(const Element& e, Renderer& renderer) {
  const auto& props = *e.props.cast<DropdownProps>();

  if (!props.open || props.choices.empty()) {
    renderer.queue_box(
        e.box(),
        theme->input_color_bg,
        theme->input_border_width,
        theme->input_color_border,
        0);

    std::string text =
        !props.open ? props.choice == -1 ? "" : props.choices[props.choice]
                    : "<none>";

    renderer.queue_text(
        e.position +
            Vecf::Constant(theme->input_border_width + theme->text_padding),
        text,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthFixed(
            e.size.x -
            2.f * (theme->input_border_width + theme->text_padding)));
    return;
  }

  Vecf offset = Vecf::Zero();

  for (int i = 0; i < props.choices.size(); i++) {
    Vecf position = e.position + offset;
    Vecf size = Vecf(
        e.size.x,
        fm->text_height(theme->text_font, theme->text_size) +
            2.f * (theme->text_padding + theme->input_border_width));

    Color bg_color;
    if (i == props.choice) {
      bg_color = theme->input_color_bg_active;
    } else {
      bg_color = theme->input_color_bg;
    }
    renderer.queue_box(
        Boxf(position, position + size),
        bg_color,
        theme->input_border_width,
        theme->input_color_border,
        0);

    renderer.queue_text(
        position +
            Vecf::Constant(theme->input_border_width + theme->text_padding),
        props.choices[i],
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthFixed(
            e.size.x -
            2.f * (theme->input_border_width + theme->text_padding)));

    offset.y += theme->input_border_width + 2 * theme->text_padding +
                fm->text_height(theme->text_font, theme->text_size);
  }
}

bool DropdownSystem::mouse_event(Element& e, const MouseEvent& event) {
  auto& props = *e.props.cast<DropdownProps>();

  if (event.action != MouseAction::Press) {
    return false;
  }
  if (!props.open) {
    props.open = true;
    return false;
  }
  if (props.choices.empty()) {
    return false;
  }

  Vecf offset = Vecf::Zero();
  int clicked = -1;

  for (int i = 0; i < props.choices.size(); i++) {
    Vecf position = e.position + offset;
    Vecf size = Vecf(
        e.size.x,
        fm->text_height(theme->text_font, theme->text_size) +
            2.f * (theme->text_padding + theme->input_border_width));

    if (Boxf(position, position + size).contains(event.position)) {
      clicked = i;
      break;
    }

    offset.y += theme->input_border_width + 2.f * theme->text_padding +
                fm->text_height(theme->text_font, theme->text_size);
  }

  if (clicked != -1) {
    if (props.choice != clicked) {
      props.changed = true;
      props.choice = clicked;
    }
    props.open = false;
    return true;
  }
  return false;
}

void DropdownSystem::focus_enter(Element& e) {
  auto& props = *e.props.cast<DropdownProps>();
  props.open = true;
}

bool DropdownSystem::focus_leave(Element& e, bool success) {
  auto& props = *e.props.cast<DropdownProps>();
  props.open = false;
  return false;
}

} // namespace datagui
