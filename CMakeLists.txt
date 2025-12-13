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
  src/log.cpp
  src/theme.cpp

  src/element/args.cpp
  src/element/key_list.cpp
  src/element/tree.cpp

  src/geometry/box.cpp
  src/geometry/camera.cpp
  src/geometry/rot.cpp
  src/geometry/vec.cpp

  src/visual/font_manager.cpp
  src/visual/image_shader.cpp
  src/visual/mesh_shader.cpp
  src/visual/gui_renderer.cpp
  src/visual/shader_utils.cpp
  src/visual/shape_2d_shader.cpp
  src/visual/shape_3d_shader.cpp
  src/visual/text_shader.cpp
  src/visual/uv_mesh_shader.cpp
  src/visual/window.cpp

  src/input/number_input.cpp
  src/input/text_selection.cpp

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
  src/system/viewport.cpp

  src/viewport/canvas2d.cpp
  src/viewport/canvas3d.cpp
  src/viewport/plotter.cpp
  src/viewport/viewport.cpp

  src/gui.cpp

  src/datapack/edit.cpp
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
  target_compile_definitions(datagui PUBLIC -DDATAGUI_DEBUG)
endif()

# Additional targets

if (NOT BUILD_ADDITIONAL_TARGETS)
    return()
endif()

add_subdirectory(examples)
add_subdirectory(test)

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
