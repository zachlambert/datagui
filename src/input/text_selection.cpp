#include "datagui/input/text_selection.hpp"
#include <GLFW/glfw3.h> // For copy/paste

namespace datagui {

std::size_t find_cursor(
    const FontStructure& font,
    const std::string& text,
    Length text_width,
    const Vec2& point) {

  auto fixed_width = std::get_if<LengthFixed>(&text_width);
  Vec2 pos;
  pos.y += font.line_height;

  std::size_t column = 0;
  bool column_found = false;

  for (std::size_t i = 0; i < text.size(); i++) {
    if (!font.char_valid(text[i])) {
      continue;
    }
    const auto& c = font.get(text[i]);

    if (!column_found && pos.x + c.advance / 2 > point.x) {
      column_found = true;
      column = i;
      if (point.y < pos.y) {
        return column;
      }
    }

    if (fixed_width && pos.x + c.advance > fixed_width->value) {
      if (!column_found) {
        column = i + 1;
      }
      if (point.y < pos.y) {
        return column;
      }
      pos.x = 0;
      pos.y += font.line_height;
      column_found = false;
    }
    pos.x += c.advance;
  }
  if (!column_found) {
    column = text.size();
  }
  return column;
}

Vec2 cursor_offset(
    const FontStructure& font,
    const std::string& text,
    Length text_width,
    std::size_t cursor) {
  auto fixed_width = std::get_if<LengthFixed>(&text_width);
  Vec2 offset;

  for (std::size_t i = 0; i < cursor; i++) {
    if (!font.char_valid(text[i])) {
      continue;
    }
    const auto& c = font.get(text[i]);
    if (fixed_width && offset.x + c.advance > fixed_width->value) {
      offset.x = 0;
      offset.y += font.line_height;
    }
    offset.x += c.advance;
  }
  return offset;
}

void selection_text_event(
    std::string& text,
    TextSelection& selection,
    bool editable,
    const TextEvent& event) {

  if (!editable) {
    return;
  }

  if (selection.span() > 0) {
    text.erase(text.begin() + selection.from(), text.begin() + selection.to());
    selection.reset(selection.from());
  }

  text.insert(text.begin() + selection.begin, event.value);
  selection.begin++;
  selection.end = selection.begin;
}

void selection_key_event(
    std::string& text,
    TextSelection& selection,
    bool editable,
    const KeyEvent& event) {

  switch (event.key) {
  case Key::Left: {
    if (event.action == KeyAction::Release) {
      break;
    }
    if (event.mod_ctrl) {
      if (selection.end == 0) {
        return;
      }
      selection.end--;
      while (selection.end != 0 && !std::isalnum(text[selection.end - 1])) {
        selection.end--;
      }
      while (selection.end != 0 && std::isalnum(text[selection.end - 1])) {
        selection.end--;
      }
      if (!event.mod_shift) {
        selection.begin = selection.end;
      }
    } else if (selection.span() > 0 && !event.mod_shift) {
      selection.reset(selection.from());
    } else if (selection.end != 0) {
      selection.end--;
      if (!event.mod_shift) {
        selection.begin = selection.end;
      }
    }
    break;
  }
  case Key::Right: {
    if (event.action == KeyAction::Release) {
      break;
    }
    if (event.mod_ctrl) {
      if (selection.end == text.size()) {
        return;
      }
      selection.end++;
      while (selection.end != text.size() &&
             !std::isalnum(text[selection.end])) {
        selection.end++;
      }
      while (selection.end != text.size() &&
             std::isalnum(text[selection.end])) {
        selection.end++;
      }
      if (!event.mod_shift) {
        selection.begin = selection.end;
      }
    } else if (selection.span() > 0 && !event.mod_shift) {
      selection.reset(selection.to());
    } else if (selection.end != text.size()) {
      selection.end++;
      if (!event.mod_shift) {
        selection.begin = selection.end;
      }
    }
    break;
  }
  case Key::Backspace: {
    if (event.action == KeyAction::Release || !editable) {
      break;
    }
    if (selection.span() > 0) {
      text.erase(
          text.begin() + selection.from(),
          text.begin() + selection.to());
      selection.reset(selection.from());

    } else if (selection.begin > 0) {
      if (event.mod_ctrl) {
        int pos = selection.begin - 1;
        while (pos != 0 && !std::isalnum(text[pos])) {
          pos--;
        }
        while (pos != 0 && std::isalnum(text[pos])) {
          pos--;
        }
        text.erase(pos, selection.begin - pos);
        selection.reset(pos);
      } else {
        selection.begin--;
        text.erase(text.begin() + selection.begin);
        selection.end = selection.begin;
      }
    }
    break;
  }
  case Key::Delete: {
    if (event.action == KeyAction::Release || !editable) {
      break;
    }
    if (selection.span() > 0) {
      text.erase(
          text.begin() + selection.from(),
          text.begin() + selection.to());
      selection.reset(selection.from());

    } else if (selection.begin < text.size()) {
      if (event.mod_ctrl) {
        int pos = selection.begin;
        while (pos != text.size() && !std::isalnum(text[pos])) {
          pos++;
        }
        while (pos != text.size() && std::isalnum(text[pos])) {
          pos++;
        }
        text.erase(selection.begin, pos - selection.begin);
      } else {
        text.erase(text.begin() + selection.begin);
      }
    }
    break;
  }
  case Key::C: {
    if (event.action != KeyAction::Release) {
      break;
    }
    if (!event.mod_ctrl || selection.span() == 0 || !event.glfw_window) {
      break;
    }
    std::string copied = text.substr(selection.from(), selection.span());
    glfwSetClipboardString((GLFWwindow*)event.glfw_window, copied.c_str());
    break;
  }
  case Key::V: {
    if (event.action != KeyAction::Release) {
      break;
    }
    if (!editable || !event.mod_ctrl || !event.glfw_window) {
      break;
    }
    const char* pasted_cstr =
        glfwGetClipboardString((GLFWwindow*)event.glfw_window);
    if (!pasted_cstr) {
      break;
    }
    const std::string pasted = pasted_cstr;

    if (selection.span() > 0) {
      text.erase(
          text.begin() + selection.from(),
          text.begin() + selection.to());
    }
    text.insert(text.begin() + selection.from(), pasted.begin(), pasted.end());
    selection.reset(selection.from() + pasted.size());
    break;
  }
  case Key::X: {
    if (event.action != KeyAction::Release) {
      break;
    }
    if (!event.mod_ctrl || selection.span() == 0 || !event.glfw_window) {
      break;
    }
    std::string copied = text.substr(selection.from(), selection.span());
    glfwSetClipboardString((GLFWwindow*)event.glfw_window, copied.c_str());
    text.erase(text.begin() + selection.from(), text.begin() + selection.to());
    selection.reset(selection.from());
    break;
  }
  default:
    break;
  }
}

void render_selection(
    const std::string& text,
    const Vec2& origin,
    const TextSelection& selection,
    const FontStructure& font,
    Color cursor_color,
    Color highlight_color,
    int cursor_width,
    Length width,
    Renderer& renderer) {

  // Render cursor only

  if (selection.span() == 0) {
    Vec2 offset = cursor_offset(font, text, width, selection.begin);
    renderer.queue_box(
        Box2(
            origin + offset - Vec2(float(cursor_width) / 2, 0),
            origin + offset + Vec2(cursor_width, font.line_height)),
        cursor_color);
    return;
  }

  auto fixed_width = std::get_if<LengthFixed>(&width);

  // Render higlighlight

  std::size_t from = selection.from();
  std::size_t to = selection.to();
  Vec2 offset = cursor_offset(font, text, width, from);
  Vec2 from_offset = offset;

  for (std::size_t i = from; i < to; i++) {
    if (!font.char_valid(text[i])) {
      continue;
    }
    const auto& c = font.get(text[i]);

    if (fixed_width && offset.x + c.advance > fixed_width->value) {
      Vec2 to_offset = offset;
      if (from == text.size()) {
        to_offset.x += c.advance;
      }
      to_offset.y += font.line_height;
      renderer.queue_box(
          Box2(origin + from_offset, origin + to_offset),
          highlight_color);

      from = i;
      offset.x = 0;
      offset.y += font.line_height;
      from_offset = offset;
    }
    offset.x += c.advance;
  }

  Vec2 to_offset = offset + Vec2(0, font.line_height);
  renderer.queue_box(
      Box2(origin + from_offset, origin + to_offset),
      highlight_color);
}

} // namespace datagui
