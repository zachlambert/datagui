#include "datagui/system/text_box.hpp"

namespace dgui {

void TextBoxSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& text_box = element.text_box();

  int text_size =
      text_box.text_size != 0 ? text_box.text_size : theme->text_size;
  const auto& font = font_registry->get_font(theme->text_font, text_size);
  state.fixed_size = font.text_size(text_box.text, LengthWrap()) +
                     Vec2::uniform(2 * theme->text_padding);
  state.dynamic_size = Vec2();
  state.floating = false;
}

void TextBoxSystem::render(ConstElementPtr element, DrawList& dl) {
  const auto& state = element.state();
  const auto& text_box = element.text_box();

  const Color& text_color =
      text_box.text_color ? *text_box.text_color : theme->text_color;
  int text_size =
      text_box.text_size != 0 ? text_box.text_size : theme->text_size;

  dl.draw_text(
      font_registry->get_font(theme->text_font, text_size),
      state.position + Vec2::uniform(theme->text_padding),
      text_color,
      LengthWrap(),
      text_box.text);
}

} // namespace dgui
