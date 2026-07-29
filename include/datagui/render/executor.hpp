#pragma once

#include "datagui/render/state/draw_list.hpp"

namespace dgui {

class ProgramRegistry;

class Executor {
public:
  void draw(const Vec2& size, ProgramRegistry& registry, const DrawList& dl);
};

} // namespace dgui
