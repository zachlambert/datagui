#pragma once

#include "datagui/element/tree.hpp"
#include "datagui/theme.hpp"
#include "datagui/visual/gui_renderer.hpp"

namespace datagui {

void layout_set_input_state(
    ElementPtr element,
    const std::shared_ptr<Theme>& theme,
    const Layout& layout,
    LayoutState& state);

void layout_set_dependent_state(
    ElementPtr element,
    const Box2& content_box,
    const std::shared_ptr<Theme>& theme,
    const Layout& layout,
    LayoutState& state);

void layout_render_scroll(
    const Box2& content_box,
    const LayoutState& state,
    const std::shared_ptr<Theme>& theme,
    GuiRenderer& renderer);

bool layout_scroll_event(
    const Box2& content_box,
    LayoutState& state,
    const ScrollEvent& event);

} // namespace datagui
