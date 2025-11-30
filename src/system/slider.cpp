#include "datagui/system/slider.hpp"

namespace datagui {

void SliderSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& slider = element.slider();

  state.fixed_size.x = slider.length;
  state.fixed_size.y = fm->text_height(theme->text_font, theme->text_size);
  state.dynamic_size = Vec2();
  state.floating = false;

  Vec2 text_size = fm->text_size(
      get_slider_text(slider),
      theme->text_font,
      theme->text_size,
      LengthWrap());
  state.fixed_size.x += theme->text_padding + text_size.x;
}

void SliderSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& slider = element.slider();

  Vec2 slider_size(slider.length, state.size.y);

  renderer.queue_box(
      Box2(state.position, slider_size),
      theme->slider_bg_color,
      theme->slider_border_width,
      theme->slider_border_color);

  const Color& slider_color =
      slider.held ? theme->slider_color_active : theme->slider_color;
  renderer.queue_box(get_slider_box(state, slider), slider_color);
}

bool SliderSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& slider = element.slider();

  if (!slider.held && event.action == MouseAction::Press) {
    Box2 slider_box = get_slider_box(state, slider);
    if (slider_box.contains(event.position)) {
      slider.held = true;
    }
    return false;
  }
  if (slider.held && event.action == MouseAction::Release) {
    slider.held = false;
    if (slider.callback) {
      slider.callback(slider.value);
      return false;
    } else {
      return true;
    }
  }
  if (event.action != MouseAction::Hold) {
    return false;
  }

  float pos =
      (event.position.x - (state.position.x + theme->slider_border_width)) /
      slider.length;
  pos = std::clamp(pos, 0.f, 1.f);

  slider.value = slider.lower + pos * (slider.upper - slider.lower);
  switch (slider.type) {
  case NumberType::I32:
    slider.value = (std::int32_t)slider.value;
    break;
  case NumberType::I64:
    slider.value = (std::int64_t)slider.value;
    break;
  case NumberType::U32:
    slider.value = (std::uint32_t)slider.value;
    break;
  case NumberType::U64:
    slider.value = (std::uint64_t)slider.value;
    break;
  case NumberType::F32:
    slider.value = (float)slider.value;
    break;
  case NumberType::F64:
    slider.value = (double)slider.value;
    break;
  case NumberType::U8:
    slider.value = (std::uint8_t)slider.value;
    break;
  }
  return false;
}

Box2 SliderSystem::get_slider_box(const State& state, const Slider& slider)
    const {
  Vec2 slider_pos = state.position + Vec2::uniform(theme->slider_border_width);
  slider_pos.x += theme->slider_width / 2;

  float inner_width =
      slider.length - theme->slider_border_width * 2 - theme->slider_width;

  float pos = 0;
  if (slider.upper > slider.lower) {
    float pos = (slider.value - slider.lower) / (slider.upper - slider.lower);
    pos = std::clamp(pos, 0.f, 1.f);
  }
  slider_pos.x += inner_width * pos;

  slider_pos.x -= theme->slider_width / 2;

  Vec2 slider_size = Vec2(
      theme->slider_width,
      fm->text_height(theme->text_font, theme->text_size));

  return Box2(slider_pos, slider_size);
}

std::string SliderSystem::get_slider_text(const Slider& slider) const {
  switch (slider.type) {
  case NumberType::I32:
    return std::to_string(std::int32_t(slider.value));
  case NumberType::I64:
    return std::to_string(std::int64_t(slider.value));
  case NumberType::U32:
    return std::to_string(std::uint32_t(slider.value));
  case NumberType::U64:
    return std::to_string(std::uint64_t(slider.value));
  case NumberType::F32:
    return std::to_string(float(slider.value));
  case NumberType::F64:
    return std::to_string(double(slider.value));
  case NumberType::U8:
    return std::to_string(std::uint8_t(slider.value));
  }
}

} // namespace datagui
