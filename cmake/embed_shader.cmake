# Wraps a shader source file in a raw string literal and emits a C++ source
# defining `extern const char <VAR>[]` in namespace <NS>.
#
# Invoked in script mode via:
#   cmake -DINPUT=.. -DOUTPUT=.. -DVAR=.. -DNS=.. -DHEADER=.. -P embed_shader.cmake

file(READ "${INPUT}" content)

file(WRITE "${OUTPUT}"
"// Generated from ${INPUT} - do not edit.\n"
"#include \"${HEADER}\"\n"
"namespace ${NS} {\n"
"extern const char ${VAR}[] = R\"GLSL(\n${content})GLSL\";\n"
"}\n")
