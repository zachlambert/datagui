#include "datagui/geometry/box.hpp"

namespace dgui {

#ifdef DGUI_DEBUG
std::ostream& operator<<(std::ostream& os, const Box2& vec) {
  os << vec.lower << ", " << vec.upper;
  return os;
}
#endif

} // namespace dgui
