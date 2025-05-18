#include "datagui/element/dropdown.hpp"

namespace datagui {

const int* DropdownSystem::visit(
    Element element,
    const std::vector<std::string>& choices,
    int initial_choice) {

  auto& data = element.data<DropdownData>();
  if (element.is_new()) {
    data.choices = choices;
    data.choice = initial_choice;
  }
  if (element.rerender()) {
    data.style.apply(res.style_manager);
  }
  if (data.changed) {
    data.changed = false;
    return &data.choice;
  }
  return nullptr;
}

void DropdownSystem::visit(
    Element element,
    const std::vector<std::string>& choices,
    const Variable<int>& choice) {

  auto& data = element.data<DropdownData>();
  if (element.is_new()) {
    data.choices = choices;
    data.choice = *choice;
  }
  if (element.rerender()) {
    data.style.apply(res.style_manager);
  }
  if (choice.modified()) {
    data.choice = *choice;
    data.changed = false;
  } else if (data.changed) {
    data.changed = false;
    choice.set(data.choice);
  }
}

void DropdownSystem::write(
    Element element,
    const std::vector<std::string>& choices,
    int choice) {
  auto& data = element.data<DropdownData>();
  if (element.is_new()) {
    data.choices = choices;
  }
  if (element.rerender()) {
    data.style.apply(res.style_manager);
  }
  data.choice = choice;
}

int read(ConstElement element) {
  const auto& data = element.data<DropdownData>();
  return data.choice;
}

void DropdownSystem::set_input_state(Element element) const {
  const auto& data = element.data<DropdownData>();
  const auto& style = data.style;

  float max_item_width = 0;
  for (const auto& choice : data.choices) {
    Vecf choice_size =
        res.font_manager.text_size(choice, style.text, LengthWrap());
    max_item_width = std::max(max_item_width, choice_size.x);
  }
  if (data.choices.empty()) {
    Vecf none_size =
        res.font_manager.text_size("<none>", style.text, LengthWrap());
    max_item_width = std::max(max_item_width, none_size.x);
  }

  element->fixed_size = Vecf::Zero();
  element->dynamic_size = Vecf::Zero();

  if (auto width = std::get_if<LengthFixed>(&style.width)) {
    element->fixed_size.x = std::min(max_item_width, width->value);
  } else if (std::get_if<LengthWrap>(&style.width)) {
    element->fixed_size.x = max_item_width;
  } else if (auto width = std::get_if<LengthDynamic>(&style.width)) {
    element->dynamic_size.x = width->weight;
  }
  element->fixed_size += style.border_width.size() + style.padding.size();
  element->fixed_size.y += res.font_manager.text_height(style.text);

  element->floating = data.open && !data.choices.empty();
}

void DropdownSystem::set_dependent_state(Element element) const {
  element->float_box = element->box();
  if (!element->floating) {
    return;
  }

  const auto& data = element.data<DropdownData>();
  const auto& style = data.style;
  element->float_box.upper.y +=
      (data.choices.size() - 1) *
      (res.font_manager.text_height(style.text) + style.padding.size().y +
       style.inner_border_width);
}

void DropdownSystem::render(ConstElement element) const {
  const auto& data = element.data<DropdownData>();
  const auto& style = data.style;

  if (!data.open || data.choices.empty()) {
    res.geometry_renderer.queue_box(
        element->box(),
        style.bg_color,
        style.border_width,
        style.border_color,
        0);

    std::string text = !data.open
                           ? data.choice == -1 ? "" : data.choices[data.choice]
                           : "<none>";

    res.text_renderer.queue_text(
        element->position + style.border_width.offset() +
            style.padding.offset(),
        text,
        style.text,
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
        res.font_manager.text_height(style.text) + style.padding.size().y +
            border->size().y);

    Color bg_color;
    if (i == data.choice) {
      bg_color = style.bg_color.multiply(style.input.active_color_factor);
    } else {
      bg_color = style.bg_color;
    }
    res.geometry_renderer.queue_box(
        Boxf(position, position + size),
        bg_color,
        *border,
        style.border_color,
        0);

    res.text_renderer.queue_text(
        position + border->offset() + style.padding.offset(),
        data.choices[i],
        style.text,
        LengthFixed(
            element->size.x - style.border_width.size().x -
            style.padding.size().x));

    offset.y += border->top + style.padding.size().y +
                res.font_manager.text_height(style.text);
  }
}

void DropdownSystem::mouse_event(Element element, const MouseEvent& event) {
  auto& data = element.data<DropdownData>();
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
        res.font_manager.text_height(style.text) + style.padding.size().y +
            border->size().y);

    if (Boxf(position, position + size).contains(event.position)) {
      clicked = i;
      break;
    }

    offset.y += border->top + style.padding.size().y +
                res.font_manager.text_height(style.text);
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

void DropdownSystem::focus_enter(Element element) {
  auto& data = element.data<DropdownData>();
  data.open = true;
}

void DropdownSystem::focus_leave(
    Element element,
    bool success,
    ConstElement new_element) {
  auto& data = element.data<DropdownData>();
  data.open = false;
}

} // namespace datagui
