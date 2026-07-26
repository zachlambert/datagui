#version 330 core

layout(location = 0) in vec2 quad_pos;
layout(location = 1) in vec3 M_col1;
layout(location = 2) in vec3 M_col2;
layout(location = 3) in vec3 M_col3;
layout(location = 4) in vec2 uv_offset;
layout(location = 5) in vec2 uv_size;

uniform mat3 PV;
out vec2 fs_uv;

void main() {
  mat3 M = mat3(M_col1, M_col2, M_col3);
  vec3 coords = PV * M * vec3(quad_pos, 1);
  gl_Position = vec4(coords.xy / coords.z, 0, 1);
  fs_uv = uv_offset + quad_pos * uv_size;
}
