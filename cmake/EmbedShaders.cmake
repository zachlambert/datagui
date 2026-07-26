# embed_shader(<name> <symbol>)
#
# Adds a build-time codegen step that wraps <input-glsl> in a raw string literal
# and compiles it into <target>, exposing it as `extern const char <symbol>[]`
# (declared in include/datagui/render/shaders.hpp). Regenerates
# whenever the shader source changes.
function(embed_shader name var)
  set(gen "${CMAKE_CURRENT_BINARY_DIR}/generated/embed_${var}.cpp")
  set(input "${PROJECT_SOURCE_DIR}/src/render/shaders/${name}")
  add_custom_command(
    OUTPUT "${gen}"
    COMMAND ${CMAKE_COMMAND}
      -DINPUT=${input}
      -DOUTPUT=${gen}
      -DVAR=${var}
      -DNS=dgui::shaders
      -DHEADER=datagui/render/shaders.hpp
      -P ${PROJECT_SOURCE_DIR}/cmake/embed_shader.cmake
    DEPENDS "${input}" ${PROJECT_SOURCE_DIR}/cmake/embed_shader.cmake
    VERBATIM)
  target_sources(datagui PRIVATE "${gen}")
endfunction()
