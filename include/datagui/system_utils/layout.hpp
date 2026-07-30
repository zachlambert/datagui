#pragma once

#include "datagui/element/tree.hpp"
#include "datagui/render/state/draw_list.hpp"
#include "datagui/theme.hpp"

namespace dgui {

void layout_set_input_state(
    ElementPtr element,
    const std::shared_ptr<Theme>& theme,
    const Layout& layout,
    LayoutState& state);

void layout_set_dependent_state(
    ElementPtr element,
    const std::shared_ptr<Theme>& theme,
    const Layout& layout,
    LayoutState& state);

void layout_render(
    const LayoutState& state,
    const std::shared_ptr<Theme>& theme,
    DrawList& dl);

bool layout_scroll_event(
    LayoutState& state,
    const ScrollEvent& event);

} // namespace dgui
