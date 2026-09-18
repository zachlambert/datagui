#pragma once

#include <string>

namespace dgui {

enum class Font { DejaVuSans, DejaVuSerif, DejaVuSansMono };

std::string lookup_font(Font font);

} // namespace dgui
