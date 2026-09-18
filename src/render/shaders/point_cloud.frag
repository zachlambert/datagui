#version 330 core

in vec3 fs_color;
out vec4 color;

void main(){
  color = vec4(fs_color, 1);
}
