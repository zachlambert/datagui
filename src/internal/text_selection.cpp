#include "datagui/internal/text_selection.hpp"
#include <GLFW/glfw3.h>

namespace datagui {

std::size_t find_cursor(
    const FontStructure& font,
    const std::string& text,
    float max_width,
    const Vecf& point) {
  bool has_max_width = max_width > 0;
  Vecf pos = Vecf::Zero();
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

    if (has_max_width && pos.x + c.advance > max_width) {
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

Vecf cursor_offset(
    const FontStructure& font,
    const std::string& text,
    float max_width,
    std::size_t cursor) {
  bool has_max_width = max_width > 0;
  Vecf offset = Vecf::Zero();

  for (std::size_t i = 0; i < cursor; i++) {
    if (!font.char_valid(text[i])) {
      continue;
    }
    const auto& c = font.get(text[i]);
    if (has_max_width && offset.x + c.advance > max_width) {
      offset.x = 0;
      offset.y += font.line_height;
    }
    offset.x += c.advance;
  }
  return offset;
}

void selection_key_event(
    std::string& text,
    TextSelection& selection,
    bool editable,
    const KeyEvent& event) {
  if (event.is_text) {
    if (!editable) {
      return;
    }

    if (selection.span() > 0) {
      text.erase(
          text.begin() + selection.from(),
          text.begin() + selection.to());
      selection.reset(selection.from());
    }

    text.insert(text.begin() + selection.begin, event.text_value);
    selection.begin++;
    selection.end = selection.begin;

  } else if (!event.key_release) {
    switch (event.key_value) {
    case KeyValue::LeftArrow: {
      if (event.key_ctrl) {
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
        if (!event.key_shift) {
          selection.begin = selection.end;
        }
      } else if (selection.span() > 0 && !event.key_shift) {
        selection.reset(selection.from());
      } else if (selection.end != 0) {
        selection.end--;
        if (!event.key_shift) {
          selection.begin = selection.end;
        }
      }
      break;
    }
    case KeyValue::RightArrow: {
      if (event.key_ctrl) {
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
        if (!event.key_shift) {
          selection.begin = selection.end;
        }
      } else if (selection.span() > 0 && !event.key_shift) {
        selection.reset(selection.to());
      } else if (selection.end != text.size()) {
        selection.end++;
        if (!event.key_shift) {
          selection.begin = selection.end;
        }
      }
      break;
    }
    case KeyValue::Backspace: {
      if (!editable) {
        break;
      }
      if (selection.span() > 0) {
        text.erase(
            text.begin() + selection.from(),
            text.begin() + selection.to());
        selection.reset(selection.from());

      } else if (selection.begin > 0) {
        if (event.key_ctrl) {
          int pos = selection.begin - 1;
          while (pos != 0 && !std::isalnum(text[pos])) {
            pos--;
          }
          while (pos != 0 && std::isalnum(text[pos])) {
            pos--;
          }
          text.erase(pos, selection.begin - pos);
          selection.reset(pos);
          if (!event.key_shift) {
            selection.begin = selection.end;
          }
        } else {
          selection.begin--;
          text.erase(text.begin() + selection.begin);
          selection.end = selection.begin;
        }
      }
      break;
    }
    default:
      break;
    }
  }
}

void render_selection(
    const Style& style,
    const FontStructure& font,
    const std::string& text,
    float max_width,
    const Vecf& origin,
    const TextSelection& selection,
    bool editable,
    GeometryRenderer& geometry_renderer) {
  // Render cursor

  if (selection.span() == 0) {
    if (!editable) {
      return;
    }
    Vecf offset = cursor_offset(font, text, max_width, selection.begin);
    geometry_renderer.queue_box(
        Boxf(
            origin + offset - Vecf(float(style.text_input.cursor_width) / 2, 0),
            origin + offset +
                Vecf(style.text_input.cursor_width, font.line_height)),
        style.text_input.cursor_color,
        0,
        Color::Black(),
        0);
    return;
  }

  bool has_max_width = max_width > 0;

  // Render higlighlight

  std::size_t from = selection.from();
  std::size_t to = selection.to();
  Vecf offset = cursor_offset(font, text, max_width, from);
  Vecf from_offset = offset;

  for (std::size_t i = from; i < to; i++) {
    if (!font.char_valid(text[i])) {
      continue;
    }
    const auto& c = font.get(text[i]);

    if (has_max_width && offset.x + c.advance > max_width) {
      Vecf to_offset = offset;
      if (from == text.size()) {
        to_offset.x += c.advance;
      }
      to_offset.y += font.line_height;
      geometry_renderer.queue_box(
          Boxf(origin + from_offset, origin + to_offset),
          style.text_input.highlight_color,
          0,
          Color::Black(),
          0);

      from = i;
      offset.x = 0;
      offset.y += font.line_height;
      from_offset = offset;
    }
    offset.x += c.advance;
  }

  Vecf to_offset = offset + Vecf(0, font.line_height);
  geometry_renderer.queue_box(
      Boxf(origin + from_offset, origin + to_offset),
      style.text_input.highlight_color,
      0,
      Color::Black(),
      0);
}

} // namespace datagui
