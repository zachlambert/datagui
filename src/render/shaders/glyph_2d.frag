#version 330 core

in vec2 fs_uv;
flat in vec4 fs_color;

uniform sampler2D tex;

out vec4 color;

void main(){
  color = vec4(fs_color.xyz, texture(tex, fs_uv).x * fs_color.a);
}
