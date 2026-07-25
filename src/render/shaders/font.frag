#version 330 core

in vec2 fs_uv;

uniform sampler2D char_texture;
out vec4 color;

void main(){
  color = vec4(1, 1, 1, texture(char_texture, fs_uv).x);
}
