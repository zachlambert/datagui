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
  src/element/button.cpp
  src/element/drop_down.cpp
  src/element/horizontal_layout.cpp
  src/element/text_box.cpp
  src/element/text_input.cpp
  src/element/vertical_layout.cpp
  src/element/window.cpp
  src/element/checkbox.cpp

  src/input/text_selection.cpp

  src/tree/tree.cpp

  src/visual/font.cpp
  src/visual/geometry_renderer.cpp
  src/visual/shader.cpp
  src/visual/text_renderer.cpp
  src/visual/window.cpp

  src/gui.cpp
)
target_link_libraries(datagui
    PUBLIC GL glfw GLEW ${CMAKE_DL_LIBS} ${FREETYPE_LIBRARIES} datapack
)
target_include_directories(datagui PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${FREETYPE_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)

# Additional targets

if (NOT BUILD_ADDITIONAL_TARGETS)
    return()
endif()

add_executable(example_tree examples/tree.cpp)
target_link_libraries(example_tree datagui)

add_executable(example_window examples/window.cpp)
target_link_libraries(example_window datagui)

add_executable(example_gui examples/gui.cpp)
target_link_libraries(example_gui datagui)

add_executable(example_gui2 examples/gui2.cpp)
target_link_libraries(example_gui2 datagui)

add_executable(example_drop_down examples/drop_down.cpp)
target_link_libraries(example_drop_down datagui)

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
