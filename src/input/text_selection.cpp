#include "datagui/input/text_selection.hpp"
#include <GLFW/glfw3.h> // For copy/paste

namespace dgui {

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
    if (event.mod.ctrl) {
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
      if (!event.mod.shift) {
        selection.begin = selection.end;
      }
    } else if (selection.span() > 0 && !event.mod.shift) {
      selection.reset(selection.from());
    } else if (selection.end != 0) {
      selection.end--;
      if (!event.mod.shift) {
        selection.begin = selection.end;
      }
    }
    break;
  }
  case Key::Right: {
    if (event.action == KeyAction::Release) {
      break;
    }
    if (event.mod.ctrl) {
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
      if (!event.mod.shift) {
        selection.begin = selection.end;
      }
    } else if (selection.span() > 0 && !event.mod.shift) {
      selection.reset(selection.to());
    } else if (selection.end != text.size()) {
      selection.end++;
      if (!event.mod.shift) {
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
      if (event.mod.ctrl) {
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
      if (event.mod.ctrl) {
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
    if (!event.mod.ctrl || selection.span() == 0 || !event.glfw_window) {
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
    if (!editable || !event.mod.ctrl || !event.glfw_window) {
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
    if (!event.mod.ctrl || selection.span() == 0 || !event.glfw_window) {
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
    const FontAtlas& font,
    Color cursor_color,
    Color highlight_color,
    int cursor_width,
    Length width,
    DrawList& dl) {

  float line_height = font.text_height();

  // Render cursor only

  if (selection.span() == 0) {
    Vec2 offset = font.cursor_offset(text, width, selection.begin);
    dl.draw_box(
        Box2(
            origin + offset - Vec2(float(cursor_width) / 2, 0),
            origin + offset + Vec2(cursor_width, line_height)),
        cursor_color);
    return;
  }

  auto fixed_width = std::get_if<LengthFixed>(&width);

  // Render higlighlight

  std::size_t from = selection.from();
  std::size_t to = selection.to();
  Vec2 offset = font.cursor_offset(text, width, from);
  Vec2 from_offset = offset;

  for (std::size_t i = from; i < to; i++) {
    float advance = font.advance(text[i]);

    if (fixed_width && offset.x + advance > fixed_width->value) {
      Vec2 to_offset = offset;
      if (from == text.size()) {
        to_offset.x += advance;
      }
      to_offset.y += line_height;
      dl.draw_box(
          Box2(origin + from_offset, origin + to_offset),
          highlight_color);

      from = i;
      offset.x = 0;
      offset.y += line_height;
      from_offset = offset;
    }
    offset.x += advance;
  }

  Vec2 to_offset = offset + Vec2(0, line_height);
  dl.draw_box(Box2(origin + from_offset, origin + to_offset), highlight_color);
}

} // namespace dgui
