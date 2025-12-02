#include "datagui/color.hpp"

namespace datapack {

DATAPACK_IMPL(datagui::Color, value, packer) {
  packer.constraint(ConstraintObjectColor());
  packer.object_begin();
  packer.value("r", value.r);
  packer.value("g", value.g);
  packer.value("b", value.b);
  packer.value("a", value.a);
  packer.object_end();
}

} // namespace datapack
