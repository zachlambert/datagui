# embed_shader(<target> <input-glsl> <symbol>)
#
# Adds a build-time codegen step that wraps <input-glsl> in a raw string literal
# and compiles it into <target>, exposing it as `extern const char <symbol>[]`
# (declared in include/datagui/render/embedded_shaders.hpp). Regenerates
# whenever the shader source changes.
function(embed_shader target input var)
  set(gen "${CMAKE_CURRENT_BINARY_DIR}/generated/embed_${var}.cpp")
  add_custom_command(
    OUTPUT "${gen}"
    COMMAND ${CMAKE_COMMAND}
      -DINPUT=${input}
      -DOUTPUT=${gen}
      -DVAR=${var}
      -DNS=dgui::shaders
      -DHEADER=datagui/render/embedded_shaders.hpp
      -P ${PROJECT_SOURCE_DIR}/cmake/embed_shader.cmake
    DEPENDS "${input}" ${PROJECT_SOURCE_DIR}/cmake/embed_shader.cmake
    VERBATIM)
  target_sources(${target} PRIVATE "${gen}")
endfunction()
