#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

out vec2 gs_point_size;
out vec3 gs_color;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float point_size;

void main(){
  mat4 VM = V * M;
  mat4 PVM = P * VM;
  gl_Position = PVM * vec4(position, 1);
  gs_color = color;

  float distance = (VM * vec4(position, 1)).z;
  vec3 size_cs = vec3(point_size, point_size, distance);
  gs_point_size = (P * vec4(size_cs, 1)).xy;
}
