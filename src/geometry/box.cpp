#include "datagui/geometry/box.hpp"

namespace datagui {

#ifdef DATAGUI_DEBUG
std::ostream& operator<<(std::ostream& os, Box2& vec) {
  os << vec.lower << ", " << vec.upper;
  return os;
}
#endif

} // namespace datagui
