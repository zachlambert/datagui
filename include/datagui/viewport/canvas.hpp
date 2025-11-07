#pragma once

#include "datagui/viewport/viewport.hpp"

namespace datagui {

class Canvas : public Viewport {
public:
private:
  void render() override;
};

}; // namespace datagui
