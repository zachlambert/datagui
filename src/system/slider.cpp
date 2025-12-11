#include "datagui/system/slider.hpp"
#include <iomanip>
#include <sstream>

namespace datagui {

void SliderSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& slider = element.slider();

  float slider_length =
      slider.length ? *slider.length : theme->slider_default_length;

  state.fixed_size = Vec2(slider_length, theme->slider_height);
  state.dynamic_size = Vec2();
  state.floating = false;

  Vec2 text_size = fm->text_size(
      get_slider_text(slider),
      theme->text_font,
      theme->text_size,
      LengthWrap());
  state.fixed_size.x += theme->text_padding +
                        std::max(text_size.x, theme->slider_default_text_size);
}

void SliderSystem::render(ConstElementPtr element, GuiRenderer& renderer) {
  const auto& state = element.state();
  const auto& slider = element.slider();

  Vec2 bg_position = state.position;
  bg_position.y += (state.size.y - theme->slider_height) / 2;

  float slider_length =
      slider.length ? *slider.length : theme->slider_default_length;
  Vec2 bg_size(slider_length, theme->slider_height);

  renderer.queue_box(
      Box2(bg_position, bg_position + bg_size),
      theme->slider_bg_color,
      theme->slider_border_width,
      theme->slider_border_color);

  Vec2 slider_pos = bg_position + Vec2::uniform(theme->slider_border_width);
  slider_pos.x += theme->slider_width / 2;
  float inner_width =
      slider_length - theme->slider_border_width * 2 - theme->slider_width;

  float slider_value = slider.held ? active_value : slider.value;
  float pos = 0;
  if (slider.upper > slider.lower) {
    pos = (slider_value - slider.lower) / (slider.upper - slider.lower);
    pos = std::clamp(pos, 0.f, 1.f);
  }
  slider_pos.x += inner_width * pos;
  slider_pos.x -= theme->slider_width / 2;

  Vec2 slider_size = Vec2(
      theme->slider_width,
      theme->slider_height - 2 * theme->slider_border_width);

  const Color& slider_color =
      slider.held ? theme->slider_color_active : theme->slider_color;

  renderer.queue_box(Box2(slider_pos, slider_pos + slider_size), slider_color);

  Vec2 text_pos = bg_position;
  text_pos.x += slider_length + theme->text_padding;
  text_pos.y += (theme->slider_height / 2) -
                (fm->text_height(theme->text_font, theme->text_size) / 2);

  renderer.queue_text(
      text_pos,
      get_slider_text(slider),
      theme->text_font,
      theme->text_size,
      theme->text_color);
}

void SliderSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& slider = element.slider();

  if (!slider.held && event.action == MouseAction::Press) {
    if (event.is_double_click) {
      slider.value = slider.initial_value;
      if (slider.callback) {
        slider.callback(slider.value);
      } else {
        element.set_dirty();
      }
      return;
    }

    float slider_length =
        slider.length ? *slider.length : theme->slider_default_length;

    Vec2 bg_position = state.position;
    bg_position.y += (state.size.y - theme->slider_height) / 2;

    Box2 slider_box;
    slider_box.lower =
        state.position + Vec2::uniform(theme->slider_border_width);
    slider_box.upper =
        state.position +
        Vec2(
            slider_length - 2 * theme->slider_border_width,
            theme->slider_height - 2 * theme->slider_border_width);

    if (slider_box.contains(event.position)) {
      active_value = slider.value;
      slider.held = true;
    }
    return;
  }
  if (slider.held && event.action == MouseAction::Release) {
    slider.held = false;
    if (!slider.always && slider.value != active_value) {
      slider.value = active_value;
      if (slider.callback) {
        slider.callback(slider.value);
      } else {
        element.set_dirty();
      }
    }
    return;
  }
  if (!slider.held || event.action != MouseAction::Hold) {
    return;
  }

  float slider_length =
      slider.length ? *slider.length : theme->slider_default_length;
  float inner_length =
      slider_length - 2 * theme->slider_border_width - theme->slider_width;
  float start_pos =
      state.position.x + theme->slider_border_width + theme->slider_width / 2;

  float pos = (event.position.x - start_pos) / inner_length;
  pos = std::clamp(pos, 0.f, 1.f);

  active_value = slider.lower + pos * (slider.upper - slider.lower);
  switch (slider.type) {
  case NumberType::I32:
    active_value = (std::int32_t)std::round(active_value);
    break;
  case NumberType::I64:
    active_value = (std::int64_t)std::round(active_value);
    break;
  case NumberType::U32:
    active_value = (std::uint32_t)std::round(active_value);
    break;
  case NumberType::U64:
    active_value = (std::uint64_t)std::round(active_value);
    break;
  case NumberType::F32:
    active_value = (float)active_value;
    break;
  case NumberType::F64:
    break;
  case NumberType::U8:
    active_value = (std::uint8_t)std::round(active_value);
    break;
  }

  if (slider.always && slider.callback) {
    slider.value = active_value;
    slider.callback(active_value);
  }
}

std::string SliderSystem::get_slider_text(const Slider& slider) const {
  double slider_value = slider.held ? active_value : slider.value;
  std::string result;
  switch (slider.type) {
  case NumberType::I32:
    return std::to_string(std::int32_t(slider_value));
  case NumberType::I64:
    return std::to_string(std::int64_t(slider_value));
  case NumberType::U32:
    return std::to_string(std::uint32_t(slider_value));
  case NumberType::U64:
    return std::to_string(std::uint64_t(slider_value));
  case NumberType::F32: {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << float(slider_value);
    return ss.str();
  }
  case NumberType::F64: {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << slider_value;
    return ss.str();
  }
  case NumberType::U8:
    return std::to_string(std::uint8_t(slider_value));
  }
  assert(false);
  return "";
}

} // namespace datagui
