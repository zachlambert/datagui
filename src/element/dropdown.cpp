#include "datagui/element/dropdown.hpp"

namespace datagui {

void DropdownSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  const auto& props = *e.props.cast<DropdownProps>();

  float max_item_width = 0;
  for (const auto& choice : props.choices) {
    Vecf choice_size = fm->text_size(choice, props.text_style, LengthWrap());
    max_item_width = std::max(max_item_width, choice_size.x);
  }
  if (props.choices.empty()) {
    Vecf none_size = fm->text_size("<none>", props.text_style, LengthWrap());
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
  e.fixed_size += props.border_width.size() + props.padding.size();
  e.fixed_size.y += fm->text_height(props.text_style);

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
  e.float_box.upper.y += (props.choices.size() - 1) *
                         (fm->text_height(props.text_style) +
                          props.padding.size().y + props.inner_border_width);
}

void DropdownSystem::render(const Element& e, Renderer& renderer) {
  const auto& props = *e.props.cast<DropdownProps>();

  if (!props.open || props.choices.empty()) {
    renderer.queue_box(
        e.box(),
        props.bg_color,
        props.border_width,
        props.border_color,
        0);

    std::string text =
        !props.open ? props.choice == -1 ? "" : props.choices[props.choice]
                    : "<none>";

    renderer.queue_text(
        e.position + props.border_width.offset() + props.padding.offset(),
        text,
        props.text_style,
        LengthFixed(
            e.size.x - props.border_width.size().x - props.padding.size().x));
    return;
  }

  BoxDims top_border = props.border_width;
  top_border.bottom = props.inner_border_width;
  BoxDims inner_border = props.border_width;
  inner_border.top = props.inner_border_width;
  inner_border.bottom = props.inner_border_width;
  BoxDims bottom_border = props.border_width;
  bottom_border.top = props.inner_border_width;

  Vecf offset = Vecf::Zero();

  for (int i = 0; i < props.choices.size(); i++) {
    const BoxDims* border;
    if (props.choices.size() == 1) {
      border = &props.border_width;
    } else if (i == 0) {
      border = &top_border;
    } else if (i == props.choices.size() - 1) {
      border = &bottom_border;
    } else {
      border = &inner_border;
    }

    Vecf position = e.position + offset;
    Vecf size = Vecf(
        e.size.x,
        fm->text_height(props.text_style) + props.padding.size().y +
            border->size().y);

    Color bg_color;
    if (i == props.choice) {
      bg_color = props.bg_color.multiply(props.input_style.active_color_factor);
    } else {
      bg_color = props.bg_color;
    }
    renderer.queue_box(
        Boxf(position, position + size),
        bg_color,
        *border,
        props.border_color,
        0);

    renderer.queue_text(
        position + border->offset() + props.padding.offset(),
        props.choices[i],
        props.text_style,
        LengthFixed(
            e.size.x - props.border_width.size().x - props.padding.size().x));

    offset.y += border->top + props.padding.size().y +
                fm->text_height(props.text_style);
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

  BoxDims top_border = props.border_width;
  top_border.bottom = props.inner_border_width;
  BoxDims inner_border = props.border_width;
  inner_border.top = props.inner_border_width;
  inner_border.bottom = props.inner_border_width;
  BoxDims bottom_border = props.border_width;
  bottom_border.top = props.inner_border_width;

  Vecf offset = Vecf::Zero();
  int clicked = -1;

  for (int i = 0; i < props.choices.size(); i++) {
    const BoxDims* border;
    if (props.choices.size() == 1) {
      border = &props.border_width;
    } else if (i == 0) {
      border = &top_border;
    } else if (i == props.choices.size() - 1) {
      border = &bottom_border;
    } else {
      border = &inner_border;
    }

    Vecf position = e.position + offset;
    Vecf size = Vecf(
        e.size.x,
        fm->text_height(props.text_style) + props.padding.size().y +
            border->size().y);

    if (Boxf(position, position + size).contains(event.position)) {
      clicked = i;
      break;
    }

    offset.y += border->top + props.padding.size().y +
                fm->text_height(props.text_style);
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
