#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 transform_col1;
layout(location = 3) in vec4 transform_col2;
layout(location = 4) in vec4 transform_col3;
layout(location = 5) in vec4 transform_col4;
layout(location = 6) in vec4 color;

out vec3 fs_normal_cs;
out vec3 fs_normal_ws;
out vec4 fs_color;
flat out int fs_instance_id;

uniform mat4 P;
uniform mat4 V;

void main(){
  mat4 M = mat4(transform_col1, transform_col2, transform_col3, transform_col4);
  mat4 VM = V * M;
  mat4 PVM = P * VM;
  gl_Position = PVM * vec4(position, 1);
  fs_normal_cs = normalize((VM * vec4(normal, 0)).xyz);
  fs_normal_ws = normalize((M * vec4(normal, 0)).xyz);
  fs_color = color;
  fs_instance_id = gl_InstanceID;
}
