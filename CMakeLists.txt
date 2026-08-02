cmake_minimum_required(VERSION 3.14)
project(datagui)

# Setup

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_ADDITIONAL_TARGETS OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(BUILD_ADDITIONAL_TARGETS ON)
endif()

find_package(datapack REQUIRED)
find_package(Freetype REQUIRED)

# Library

add_library(datagui SHARED
  src/color.cpp
  src/font.cpp
  src/log.cpp
  src/theme.cpp

  src/element/args.cpp
  src/element/key_list.cpp
  src/element/layer_manager.cpp
  src/element/system.cpp
  src/element/tree.cpp

  src/geometry/box.cpp
  src/geometry/camera.cpp
  src/geometry/mat.cpp
  src/geometry/rot.cpp
  src/geometry/vec.cpp

  src/asset/image.cpp
  src/asset/mesh.cpp
  src/asset/point_cloud.cpp
  src/asset/uv_mesh.cpp

  src/input/number_input.cpp
  src/input/text_selection.cpp

  src/render/lookup/ansi_colors.cpp
  src/render/lookup/color_map.cpp

  src/render/program/font_program.cpp
  src/render/program/glyph_2d_program.cpp
  src/render/program/image_2d_program.cpp
  src/render/program/mesh_program.cpp
  src/render/program/point_cloud_program.cpp
  src/render/program/shape_2d_program.cpp
  src/render/program/shape_3d_program.cpp

  src/render/state/draw_list.cpp
  src/render/state/scene_2d.cpp
  src/render/state/scene_3d.cpp
  src/render/state/shape_2d_instance.cpp
  src/render/state/shape_3d_instance.cpp
  src/render/state/shape_3d_vertex.cpp

  src/render/executor.cpp
  src/render/font_atlas.cpp
  src/render/font_registry.cpp
  src/render/program_registry.cpp
  src/render/shaders.cpp
  src/render/window.cpp


  src/system_utils/layout.cpp

  src/system/button.cpp
  src/system/checkbox.cpp
  src/system/collapsable.cpp
  src/system/color_picker.cpp
  src/system/dropdown.cpp
  src/system/group.cpp
  src/system/popup.cpp
  src/system/select.cpp
  src/system/slider.cpp
  src/system/split.cpp
  src/system/tabs.cpp
  src/system/text_box.cpp
  src/system/text_input.cpp
  src/system/viewport_ptr.cpp

  src/viewport/canvas2d.cpp
  src/viewport/canvas3d.cpp
  # src/viewport/plotter.cpp

  src/gui.cpp

  src/datapack/common.cpp
  src/datapack/reader.cpp
  src/datapack/writer.cpp
)
target_link_libraries(datagui
    PUBLIC GL glfw GLEW ${CMAKE_DL_LIBS} ${FREETYPE_LIBRARIES} datapack
)
target_include_directories(datagui PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${FREETYPE_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  target_compile_definitions(datagui PUBLIC -DDGUI_DEBUG)
endif()

# Embed shader sources into the library at build time

include(cmake/EmbedShaders.cmake)
embed_shader(shape_2d.vert shape_2d_vs)
embed_shader(shape_2d.frag shape_2d_fs)
embed_shader(glyph_2d.vert glyph_2d_vs)
embed_shader(glyph_2d.frag glyph_2d_fs)
embed_shader(image_2d.vert image_2d_vs)
embed_shader(image_2d.frag image_2d_fs)
embed_shader(font.vert font_vs)
embed_shader(font.frag font_fs)
embed_shader(mesh.vert mesh_vs)
embed_shader(mesh.frag mesh_fs)
embed_shader(point_cloud.vert point_cloud_vs)
embed_shader(point_cloud.geom point_cloud_gs)
embed_shader(point_cloud.frag point_cloud_fs)
embed_shader(shape_3d.vert shape_3d_vs)
embed_shader(shape_3d.frag shape_3d_fs)

# Additional targets

if (NOT BUILD_ADDITIONAL_TARGETS)
    return()
endif()

add_subdirectory(test)
add_subdirectory(examples)

# Create install target

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/dataguiConfigVersion.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS datagui
    EXPORT dataguiTargets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/dataguiConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/dataguiConfig.cmake"
    INSTALL_DESTINATION lib/cmake/datagui
)

install(EXPORT dataguiTargets DESTINATION lib/cmake/datagui)
install(FILES "${PROJECT_BINARY_DIR}/dataguiConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/dataguiConfig.cmake"
        DESTINATION lib/cmake/datagui)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)
