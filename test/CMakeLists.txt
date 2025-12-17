include(FetchContent)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
include(GoogleTest)
set(INSTALL_GTEST OFF)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

function(create_test directory name)
  set(full_name test_${directory}_${name})
  add_executable(${full_name} ${directory}/${name}.cpp)
  target_link_libraries(${full_name} datagui GTest::gtest_main)
  gtest_discover_tests(${full_name})
endfunction()

function(create_test_program directory name)
  set(full_name test_${directory}_${name})
  add_executable(${full_name} ${directory}/${name}.cpp)
  target_link_libraries(${full_name} datagui)
endfunction()

create_test(element tree)
create_test(element unique_any)
create_test(element vector_map)

create_test(geometry vec)
create_test(geometry mat)
create_test(geometry rot)

create_test_program(visual window)
create_test_program(visual shape_2d_shader)
create_test_program(visual text_2d_shader)
create_test_program(visual image_shader)
create_test_program(visual mesh_shader)
create_test_program(visual uv_mesh_shader)
create_test_program(visual point_cloud_shader)
