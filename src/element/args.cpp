#include "datagui/element/args.hpp"
#include "datagui/element/element.hpp"

namespace datagui {

void Args::apply(ElementPtr element) {
  switch (element.type()) {
  case Type::Button: {
    auto& button = element.button();
    text_size_.consume(button.text_size);
    text_color_.consume(button.text_color);
    break;
  }
  case Type::Checkbox: {
    auto& checkbox = element.checkbox();
    label_.consume(checkbox.label);
    break;
  }
  case Type::Collapsable: {
    auto& collapsable = element.collapsable();
    header_color_.consume(collapsable.header_color);
    bg_color_.consume(collapsable.bg_color);
    border_.consume(collapsable.border);
    layout_.consume(collapsable.layout);
    width_.consume(collapsable.width);
    height_.consume(collapsable.height);
    break;
  }
  case Type::ColorPicker: {
    auto& color_picker = element.color_picker();
    label_.consume(color_picker.label);
    always_.consume(color_picker.always);
    break;
  }
  case Type::Dropdown: {
    break;
  }
  case Type::Group: {
    auto& group = element.group();
    bg_color_.consume(group.bg_color);
    border_.consume(group.border);
    layout_.consume(group.layout);
    width_.consume(group.width);
    height_.consume(group.height);
    break;
  }
  case Type::Popup: {
    auto& popup = element.popup();
    header_color_.consume(popup.header_color);
    bg_color_.consume(popup.bg_color);
    layout_.consume(popup.layout);
    break;
  }
  case Type::Select: {
    auto& select = element.select();
    label_.consume(select.label);
    break;
  }
  case Type::Slider: {
    auto& slider = element.slider();
    label_.consume(slider.label);
    always_.consume(slider.always);
    slider_length_.consume(slider.length);
    break;
  }
  case Type::Split: {
    auto& split = element.split();
    split_fixed_.consume(split.fixed);
    width_.consume(split.width);
    height_.consume(split.height);
    break;
  }
  case Type::Tabs: {
    break;
  }
  case Type::TextBox: {
    auto& text_box = element.text_box();
    text_size_.consume(text_box.text_size);
    text_color_.consume(text_box.text_color);
    break;
  }
  case Type::TextInput: {
    auto& text_input = element.text_input();
    label_.consume(text_input.label);
    text_input_width_.consume(text_input.width);
    break;
  }
  case Type::ViewportPtr: {
    break;
  }
  }
}

} // namespace datagui
