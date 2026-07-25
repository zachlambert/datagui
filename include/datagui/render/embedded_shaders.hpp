#pragma once

namespace dgui {
namespace shaders {

// Shader sources embedded at build time from src/render/shaders/*.
// See cmake/EmbedShaders.cmake.
extern const char shape_2d_vs[];
extern const char shape_2d_fs[];

} // namespace shaders
} // namespace dgui
