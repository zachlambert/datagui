#include "datagui/system_utils/layout.hpp"

namespace datagui {

void layout_set_input_state(
    ElementPtr element,
    const std::shared_ptr<Theme>& theme,
    const Layout& layout,
    LayoutState& state) {

  bool row_major = (layout.cols > 0);

  if (row_major) {
    assert(layout.cols > 0);
    state.col_input_sizes.resize(layout.cols);
    state.row_input_sizes.clear();
  } else {
    assert(layout.rows > 0);
    state.row_input_sizes.resize(layout.rows);
    state.col_input_sizes.clear();
  }

  auto child = element.child();
  std::size_t i = 0;
  std::size_t j = 0;
  while (child) {
    if (child.state().float_only) {
      child = child.next();
      continue;
    }
    if (j == 0 && row_major) {
      state.row_input_sizes.push_back({0, 0});
    } else if (i == 0 && !row_major) {
      state.col_input_sizes.push_back({0, 0});
    }

    auto& row_size = state.row_input_sizes[i];
    auto& col_size = state.col_input_sizes[j];
    const auto& c_state = child.state();

    col_size.fixed = std::max(col_size.fixed, c_state.fixed_size.x);
    col_size.dynamic = std::max(col_size.dynamic, c_state.dynamic_size.x);
    row_size.fixed = std::max(row_size.fixed, c_state.fixed_size.y);
    row_size.dynamic = std::max(row_size.dynamic, c_state.dynamic_size.y);

    if (row_major) {
      j++;
      if (j == layout.cols) {
        j = 0;
        i++;
        if (layout.rows > 0 && i == layout.rows) {
          break;
        }
      }
    } else {
      i++;
      if (i == layout.rows) {
        i = 0;
        j++;
        if (layout.cols > 0 && j == layout.cols) {
          break;
        }
      }
    }
    child = child.next();
  }

  float outer_padding = layout.tight ? 0.f : theme->layout_outer_padding;
  float inner_padding = layout.tight ? 0.f : theme->layout_inner_padding;

  state.content_fixed_size = Vec2::uniform(outer_padding * 2);
  state.content_dynamic_size = Vec2();
  for (const auto& sizes : state.col_input_sizes) {
    state.content_fixed_size.x += sizes.fixed;
    state.content_dynamic_size.x += sizes.dynamic;
  }
  for (const auto& sizes : state.row_input_sizes) {
    state.content_fixed_size.y += sizes.fixed;
    state.content_dynamic_size.y += sizes.dynamic;
  }

  if (state.col_input_sizes.size() > 0) {
    state.content_fixed_size.x +=
        (state.col_input_sizes.size() - 1) * inner_padding;
  }
  if (state.row_input_sizes.size() > 0) {
    state.content_fixed_size.y +=
        (state.row_input_sizes.size() - 1) * inner_padding;
  }
}

void layout_set_dependent_state(
    ElementPtr element,
    const Box2& content_box,
    const std::shared_ptr<Theme>& theme,
    const Layout& layout,
    LayoutState& state) {

  std::vector<float> col_sizes(state.col_input_sizes.size());
  {
    float size = content_box.size().x;
    float content_size = state.content_fixed_size.x;
    float available = std::max(size - content_size, 0.f);
    float dynamic_size = state.content_dynamic_size.x;
    state.content_overrun.x = std::max(content_size - size, 0.f);

    for (std::size_t j = 0; j < col_sizes.size(); j++) {
      col_sizes[j] = state.col_input_sizes[j].fixed;
      if (dynamic_size > 0) {
        col_sizes[j] +=
            available * state.col_input_sizes[j].dynamic / dynamic_size;
      }
    }
  }

  std::vector<float> row_sizes(state.row_input_sizes.size());
  {
    float size = content_box.size().y;
    float content_size = state.content_fixed_size.y;
    float available = std::max(size - content_size, 0.f);
    float dynamic_size = state.content_dynamic_size.x;
    state.content_overrun.y = std::max(content_size - size, 0.f);

    for (std::size_t i = 0; i < row_sizes.size(); i++) {
      row_sizes[i] = state.row_input_sizes[i].fixed;
      if (dynamic_size > 0) {
        row_sizes[i] +=
            available * state.row_input_sizes[i].dynamic / dynamic_size;
      }
    }
  }

  bool row_major = (layout.cols > 0);

  float outer_padding = layout.tight ? 0.f : theme->layout_outer_padding;
  float inner_padding = layout.tight ? 0.f : theme->layout_inner_padding;

  auto child = element.child();
  std::size_t i = 0;
  std::size_t j = 0;
  Vec2 origin =
      content_box.lower - state.scroll_pos + Vec2::uniform(outer_padding);
  Vec2 offset;

  while (child) {
    if (child.state().float_only) {
      child.state().position = content_box.lower;
      child = child.next();
      continue;
    }
    const Vec2 cell_size = Vec2(col_sizes[j], row_sizes[i]);
    const Vec2& fixed_size = child.state().fixed_size;
    const Vec2& dynamic_size = child.state().dynamic_size;
    Vec2& size = child.state().size;

    if (dynamic_size.x > 0) {
      size.x = cell_size.x;
    } else {
      size.x = fixed_size.x;
    }
    if (dynamic_size.y > 0) {
      size.y = cell_size.y;
    } else {
      size.y = fixed_size.y;
    }

    Vec2& position = child.state().position;

    switch (layout.x_alignment) {
    case XAlignment::Left:
      position.x = offset.x;
      break;
    case XAlignment::Center:
      position.x = offset.x + (cell_size.x - size.x) / 2;
      break;
    case XAlignment::Right:
      position.x = offset.x + (cell_size.x - size.x);
      break;
    }
    switch (layout.y_alignment) {
    case YAlignment::Top:
      position.y = offset.y;
      break;
    case YAlignment::Center:
      position.y = offset.y + (cell_size.y - size.y) / 2;
      break;
    case YAlignment::Bottom:
      position.y = offset.y + (cell_size.y - size.y);
      break;
    }
    position += origin;

    if (row_major) {
      j++;
      offset.x += cell_size.x + inner_padding;
      if (j == layout.cols) {
        offset.y += cell_size.y + inner_padding;
        offset.x = 0;
        j = 0;
        i++;
        if (i == layout.rows) {
          break;
        }
      }
    } else {
      i++;
      offset.y += cell_size.y + inner_padding;
      if (i == layout.rows) {
        offset.x += cell_size.x + inner_padding;
        offset.y = 0;
        i = 0;
        j++;
        if (j == layout.cols) {
          break;
        }
      }
    }
    child = child.next();
  }
  while (child) {
    child.state().hidden = true;
    child = child.next();
  }
}

void layout_render_scroll(
    const Box2& content_box,
    const LayoutState& state,
    const std::shared_ptr<Theme>& theme,
    Renderer& renderer) {

  if (state.content_overrun.x > 0) {
    Vec2 origin = content_box.lower;
    Vec2 size = content_box.size();
    Box2 bg;
    Box2 fg;

    bg.lower.x = origin.x;
    bg.upper.x = origin.x + size.x;
    bg.lower.y = origin.y + size.y - theme->scroll_bar_width;
    bg.upper.y = origin.y + size.y;

    float ratio = size.x / (state.content_overrun.x + size.x);
    float location = state.scroll_pos.x / (state.content_overrun.x + size.x);

    fg = bg;
    fg.lower.x = origin.x + location * size.x;
    fg.upper.x = origin.x + (location + ratio) * size.x;

    renderer.queue_box(bg, theme->scroll_bar_bg);
    renderer.queue_box(fg, theme->scroll_bar_fg);
  }

  if (state.content_overrun.y > 0) {
    Vec2 origin = content_box.lower;
    Vec2 size = content_box.size();
    Box2 bg;
    Box2 fg;

    bg.lower.x = origin.x + size.x - theme->scroll_bar_width;
    bg.upper.x = origin.x + size.x;
    bg.lower.y = origin.y;
    bg.upper.y = origin.y + size.y;

    float ratio = size.y / (state.content_overrun.y + size.y);
    float location = state.scroll_pos.y / (state.content_overrun.y + size.y);

    fg = bg;
    fg.lower.y = origin.y + location * size.y;
    fg.upper.y = origin.y + (location + ratio) * size.y;

    renderer.queue_box(bg, theme->scroll_bar_bg);
    renderer.queue_box(fg, theme->scroll_bar_fg);
  }
}

bool layout_scroll_event(
    const Box2& content_box,
    LayoutState& state,
    const ScrollEvent& event) {
  if (!content_box.contains(event.position)) {
    return false;
  }

  Vec2 new_pos = state.scroll_pos;
  if (state.content_overrun.x > 0) {
    new_pos.x = std::clamp(
        state.scroll_pos.x + event.amount,
        0.f,
        state.content_overrun.x);
  }
  if (state.content_overrun.y > 0) {
    new_pos.y = std::clamp(
        state.scroll_pos.y + event.amount,
        0.f,
        state.content_overrun.y);
  }

  bool changed = (new_pos != state.scroll_pos);
  state.scroll_pos = new_pos;
  return changed;
}

} // namespace datagui
