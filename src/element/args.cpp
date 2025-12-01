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
    break;
  }
  case Type::ColorPicker: {
    auto& color_picker = element.color_picker();
    always_.consume(color_picker.always);
    break;
  }
  case Type::Dropdown: {
    break;
  }
  case Type::Floating: {
    auto& floating = element.floating();
    header_color_.consume(floating.header_color);
    bg_color_.consume(floating.bg_color);
    break;
  }
  case Type::Labelled: {
    break;
  }
  case Type::Section: {
    auto& section = element.section();
    header_color_.consume(section.header_color);
    bg_color_.consume(section.bg_color);
    border_.consume(section.border);
    tight_.consume(section.tight);
    break;
  }
  case Type::Series: {
    auto& series = element.series();
    bg_color_.consume(series.bg_color);
    border_.consume(series.border);
    tight_.consume(series.tight);
    series_.direction.consume(series.direction);
    series_.alignment.consume(series.alignment);
    series_.length.consume(series.length);
    series_.width.consume(series.width);
    break;
  }
  case Type::Slider: {
    auto& slider = element.slider();
    always_.consume(slider.always);
    slider_.length.consume(slider.length);
    break;
  }
  case Type::TextBox: {
    auto& text_box = element.text_box();
    text_size_.consume(text_box.text_size);
    text_color_.consume(text_box.text_color);
    break;
  }
  case Type::TextInput: {
    break;
  }
  case Type::ViewportPtr: {
    break;
  }
  }
}

} // namespace datagui
