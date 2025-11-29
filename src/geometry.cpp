#include "datagui/geometry.hpp"

namespace datagui {

#ifdef DATAGUI_DEBUG
std::ostream& operator<<(std::ostream& os, Vecf& vec) {
  os << "[ " << vec.x << ", " << vec.y << " ]";
  return os;
}
#endif

} // namespace datagui
