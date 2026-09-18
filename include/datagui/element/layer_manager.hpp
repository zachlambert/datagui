#pragma once

#include "datagui/element/tree.hpp"

namespace dgui {

struct Layer {
  ElementPtr element;
  bool is_content = false;
};

class LayerManager {
public:
  void visit(ElementPtr root);

  const std::vector<Layer>& layers() const {
    return layers_;
  }

private:
  std::vector<Layer> layers_;
};

} // namespace dgui
