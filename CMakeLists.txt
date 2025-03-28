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
    src/element/checkbox.cpp
    src/element/linear_layout.cpp
    src/element/selection.cpp
    src/element/text.cpp
    src/element/text_input.cpp
    src/element/wrapper.cpp
    src/internal/geometry_renderer.cpp
    # src/internal/reader.cpp
    src/internal/text_selection.cpp
    src/internal/shader.cpp
    src/internal/text.cpp
    src/internal/text_renderer.cpp
    src/internal/tree.cpp
    src/window.cpp
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

add_executable(example1 examples/example1.cpp)
target_link_libraries(example1 datagui)

# add_executable(example2 examples/example2.cpp)
# target_link_libraries(example2 datagui)

add_executable(example3 examples/example3.cpp)
target_link_libraries(example3 datagui)

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
