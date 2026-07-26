#version 330 core

layout(location = 0) in vec2 quad_pos;
layout(location = 1) in vec2 uv;

uniform mat3 PVM;
out vec2 fs_uv;

void main() {
  vec3 coords = PVM * vec3(quad_pos, 1);
  gl_Position = vec4(coords.xy / coords.z, 0, 1);
  fs_uv = uv;
}
