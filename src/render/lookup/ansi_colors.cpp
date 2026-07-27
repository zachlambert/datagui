#include "datagui/render/lookup/ansi_colors.hpp"
#include <charconv>
#include <cstring>

namespace dgui {

namespace {

enum COLOR_FG {
  FG_BLACK = 30,
  FG_RED = 31,
  FG_GREEN = 32,
  FG_YELLOW = 33,
  FG_BLUE = 34,
  FG_MAGENTA = 35,
  FG_CYAN = 36,
  FG_WHITE = 37,
};

// https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
const Color BLACK = {0, 0, 0, 1};
const Color RED = {196.f / 255, 0, 0, 1};
const Color GREEN = {0, 196.f / 255, 0, 1};
const Color YELLOW = {192.f / 255, 126.f / 255, 0, 1};
const Color BLUE = {0, 0, 196.f / 255, 1};
const Color MAGENTA = {196.f / 255, 0, 196.f / 255, 1};
const Color CYAN = {0, 196.f / 255, 196.f / 255, 1};
const Color WHITE = {196.f / 255, 196.f / 255, 196.f / 255, 1};

void apply_code(int code, const Color& fg_color_default, Color& fg_color) {
  switch (code) {
    case 0:
      fg_color = fg_color_default;
    case FG_BLACK:
      fg_color = BLACK;
      break;
    case FG_RED:
      fg_color = RED;
      break;
    case FG_GREEN:
      fg_color = GREEN;
      break;
    case FG_YELLOW:
      fg_color = YELLOW;
      break;
    case FG_BLUE:
      fg_color = BLUE;
      break;
    case FG_MAGENTA:
      fg_color = MAGENTA;
      break;
    case FG_CYAN:
      fg_color = CYAN;
      break;
    case FG_WHITE:
      fg_color = WHITE;
      break;
    default:
      break;
  }
}

} // namespace

size_t ansi_sequence_match(
    const char* text,
    size_t length,
    const Color& default_color,
    Color& color) {

  if (length < 2 || std::strncmp(text, "\e[", 2) != 0) {
    return 0;
  }

  int start = 2;
  int i = start;
  while (i < length) {
    if (text[i] != ';' && text[i] != 'm') {
      i++;
      continue;
    }

    int code;
    auto [_, ec] = std::from_chars(text + start, text + i, code);
    if (ec == std::errc{}) {
      apply_code(code, default_color, color);
    }

    if (text[i] == 'm') {
      i++;
      break;
    }
    i++;
    start = i;
  }
  return i;
}

} // namespace dgui
