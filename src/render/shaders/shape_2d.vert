#version 330 core

layout(location = 0) in vec2 vertex_pos;
layout(location = 1) in vec3 M_col1;
layout(location = 2) in vec3 M_col2;
layout(location = 3) in vec3 M_col3;
layout(location = 4) in vec4 color;
layout(location = 5) in vec4 border_color;
layout(location = 6) in vec2 border_width;
layout(location = 7) in vec2 radius;

uniform mat3 PV;

out vec2 fs_position_ms;

flat out vec4 fs_color;
flat out vec4 fs_border_color;
flat out vec2 fs_border_width;
flat out vec2 fs_radius;

void main() {
  mat3 M = mat3(M_col1, M_col2, M_col3);
  vec3 coords = PV * M * vec3(vertex_pos, 1);
  gl_Position = vec4(coords.xy / coords.z, 0, 1);

  fs_position_ms = vertex_pos;
  fs_color = color;
  fs_border_color = border_color;
  fs_border_width = border_width;
  fs_radius = radius;
}
