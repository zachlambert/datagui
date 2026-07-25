#pragma once

#include "datagui/render/draw_list.hpp"

namespace dgui {

class ProgramRegistry;

class Executor {
  void draw(const Vec2& size, ProgramRegistry& registry, const DrawList& dl);
};

} // namespace dgui
