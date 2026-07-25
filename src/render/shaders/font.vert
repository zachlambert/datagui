#version 330 core

// Input vertex data: position and normal
layout(location = 0) in vec2 vertex_pos;
layout(location = 1) in vec2 uv;

out vec2 fs_uv;

void main(){
  gl_Position = vec4(vertex_pos.x, vertex_pos.y, 0, 1);
  fs_uv = uv;
}
