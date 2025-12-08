#include "datagui/system/text_box.hpp"

namespace datagui {

void TextBoxSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& text_box = element.text_box();

  int text_size =
      text_box.text_size != 0 ? text_box.text_size : theme->text_size;
  state.fixed_size =
      fm->text_size(text_box.text, theme->text_font, text_size, LengthWrap()) +
      Vec2::uniform(2 * theme->text_padding);
  state.dynamic_size = Vec2();
  state.floating = false;
}

void TextBoxSystem::render(ConstElementPtr element, GuiRenderer& renderer) {
  const auto& state = element.state();
  const auto& text_box = element.text_box();

  Box2 mask;
  mask.lower = state.position + Vec2::uniform(theme->text_padding);
  mask.upper = state.position + state.size - Vec2::uniform(theme->text_padding);

  const Color& text_color =
      text_box.text_color ? *text_box.text_color : theme->text_color;
  int text_size =
      text_box.text_size != 0 ? text_box.text_size : theme->text_size;

  renderer.push_mask(mask);
  renderer.queue_text(
      state.position + Vec2::uniform(theme->text_padding),
      text_box.text,
      theme->text_font,
      text_size,
      text_color,
      LengthWrap());
  renderer.pop_mask();
}

} // namespace datagui
