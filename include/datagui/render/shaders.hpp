#pragma once

namespace dgui {

namespace shaders {

extern const char shape_2d_vs[];
extern const char shape_2d_fs[];

extern const char glyph_2d_vs[];
extern const char glyph_2d_fs[];

extern const char image_2d_vs[];
extern const char image_2d_fs[];

extern const char font_vs[];
extern const char font_fs[];

extern const char mesh_vs[];
extern const char mesh_fs[];

extern const char point_cloud_vs[];
extern const char point_cloud_gs[];
extern const char point_cloud_fs[];

extern const char shape_3d_vs[];
extern const char shape_3d_fs[];

} // namespace shaders

unsigned int compile_program_vf(const char* vs_code, const char* fs_code);

unsigned int compile_program_vfg(
    const char* vs_code,
    const char* fs_code,
    const char* gs_code);

} // namespace dgui
