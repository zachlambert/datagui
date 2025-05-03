#include "datagui/element/drop_down.hpp"

namespace datagui {

const int* DropDownSystem::visit(
    Element element,
    const std::vector<std::string>& choices,
    int initial_choice,
    const SetDropDownStyle& set_style) {

  auto& data = element.data<DropDownData>();
  if (element.is_new()) {
    data.choices = choices;
    data.choice = initial_choice;
  }
  if (element.rerender()) {
    if (set_style) {
      set_style(data.style);
    }
  }
  if (data.changed) {
    data.changed = false;
    return &data.choice;
  }
  return nullptr;
}

void DropDownSystem::visit(
    Element element,
    const std::vector<std::string>& choices,
    const Variable<int>& choice,
    const SetDropDownStyle& set_style) {

  auto& data = element.data<DropDownData>();
  if (element.is_new()) {
    data.choices = choices;
    data.choice = *choice;
  }
  if (element.rerender()) {
    if (set_style) {
      set_style(data.style);
    }
  }
  if (choice.modified()) {
    data.choice = *choice;
    data.changed = false;
  } else if (data.changed) {
    data.changed = false;
    choice.set(data.choice);
  }
}

void DropDownSystem::set_layout_input(Element element) const {
  const auto& data = element.data<DropDownData>();
  const auto& style = data.style;

  float fixed_inner_width = 0;
  for (const auto& choice : data.choices) {
    Vecf choice_size =
        font_manager.text_size(choice, style, style.content_width);
    fixed_inner_width = std::max(fixed_inner_width, choice_size.x);
  }
  if (data.choices.empty()) {
    Vecf none_size =
        font_manager.text_size("<none>", style, style.content_width);
    fixed_inner_width = std::max(fixed_inner_width, none_size.x);
  }

  element->fixed_size = style.border_width.size();
  element->fixed_size.x += fixed_inner_width + style.padding.size().x;
  element->dynamic_size = Vecf::Zero();

  if (auto width = std::get_if<LengthDynamic>(&style.content_width)) {
    element->dynamic_size.x = width->weight;
  }

  element->fixed_size.y += font_manager.text_height(style);
  element->fixed_size.y += style.padding.size().y;

  element->layer_offset = data.open && !data.choices.empty() ? 1 : 0;
}

void DropDownSystem::set_hitbox(Element element) const {
  element->hitbox = element->box();
  if (element->layer_offset == 0) {
    return;
  }

  const auto& data = element.data<DropDownData>();
  const auto& style = data.style;
  element->hitbox.upper.y +=
      (data.choices.size() - 1) *
      (font_manager.text_height(style) + style.padding.size().y +
       style.inner_border_width);
}

void DropDownSystem::render(ConstElement element) const {
  const auto& data = element.data<DropDownData>();
  const auto& style = data.style;

  if (!data.open || data.choices.empty()) {
    geometry_renderer.queue_box(
        element->box(),
        style.bg_color,
        style.border_width,
        style.border_color,
        0);

    std::string text = !data.open
                           ? data.choice == -1 ? "" : data.choices[data.choice]
                           : "<none>";

    text_renderer.queue_text(
        element->position + style.border_width.offset() +
            style.padding.offset(),
        text,
        style,
        LengthFixed(
            element->size.x - style.border_width.size().x -
            style.padding.size().x));
    return;
  }

  BoxDims top_border = style.border_width;
  top_border.bottom = style.inner_border_width;
  BoxDims inner_border = style.border_width;
  inner_border.top = style.inner_border_width;
  inner_border.bottom = style.inner_border_width;
  BoxDims bottom_border = style.border_width;
  bottom_border.top = style.inner_border_width;

  Vecf offset = Vecf::Zero();

  for (int i = 0; i < data.choices.size(); i++) {
    const BoxDims* border;
    if (data.choices.size() == 1) {
      border = &style.border_width;
    } else if (i == 0) {
      border = &top_border;
    } else if (i == data.choices.size() - 1) {
      border = &bottom_border;
    } else {
      border = &inner_border;
    }

    Vecf position = element->position + offset;
    Vecf size = Vecf(
        element->size.x,
        font_manager.text_height(style) + style.padding.size().y +
            border->size().y);

    geometry_renderer.queue_box(
        Boxf(position, position + size),
        i == data.choice ? style.choice_color : style.bg_color,
        *border,
        style.border_color,
        0);

    text_renderer.queue_text(
        position + border->offset() + style.padding.offset(),
        data.choices[i],
        style,
        LengthFixed(
            element->size.x - style.border_width.size().x -
            style.padding.size().x));

    offset.y +=
        border->top + style.padding.size().y + font_manager.text_height(style);
  }
}

void DropDownSystem::mouse_event(Element element, const MouseEvent& event) {
  auto& data = element.data<DropDownData>();
  const auto& style = data.style;

  if (event.action != MouseAction::Press) {
    return;
  }
  if (!data.open) {
    data.open = true;
    return;
  }
  if (data.choices.empty()) {
    return;
  }

  BoxDims top_border = style.border_width;
  top_border.bottom = style.inner_border_width;
  BoxDims inner_border = style.border_width;
  inner_border.top = style.inner_border_width;
  inner_border.bottom = style.inner_border_width;
  BoxDims bottom_border = style.border_width;
  bottom_border.top = style.inner_border_width;

  Vecf offset = Vecf::Zero();
  int clicked = -1;

  for (int i = 0; i < data.choices.size(); i++) {
    const BoxDims* border;
    if (data.choices.size() == 1) {
      border = &style.border_width;
    } else if (i == 0) {
      border = &top_border;
    } else if (i == data.choices.size() - 1) {
      border = &bottom_border;
    } else {
      border = &inner_border;
    }

    Vecf position = element->position + offset;
    Vecf size = Vecf(
        element->size.x,
        font_manager.text_height(style) + style.padding.size().y +
            border->size().y);

    if (Boxf(position, position + size).contains(event.position)) {
      clicked = i;
      break;
    }

    offset.y +=
        border->top + style.padding.size().y + font_manager.text_height(style);
  }

  if (clicked != -1) {
    if (data.choice != clicked) {
      data.changed = true;
      data.choice = clicked;
    }
    data.open = false;
    element.trigger();
  }
}

void DropDownSystem::focus_enter(Element element) {
  auto& data = element.data<DropDownData>();
  data.open = true;
}

void DropDownSystem::focus_leave(
    Element element,
    bool success,
    ConstElement new_element) {
  auto& data = element.data<DropDownData>();
  data.open = false;
}

} // namespace datagui
