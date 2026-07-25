#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

uniform mat3 PV;
out vec2 fs_uv;

void main() {
  vec3 coords = PV * vec3(position, 1);
  gl_Position = vec4(coords.xy / coords.z, 0, 1);
  fs_uv = uv;
}
