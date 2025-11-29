#include "datagui/geometry/vec.hpp"

namespace datagui {

#ifdef DATAGUI_DEBUG

std::ostream& operator<<(std::ostream& os, const Vec2& vec) {
  os << "[ " << vec.x << ", " << vec.y << " ]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Vec3& vec) {
  os << "[ " << vec.x << ", " << vec.y << ", " << vec.z << " ]";
  return os;
}

#endif

} // namespace datagui
