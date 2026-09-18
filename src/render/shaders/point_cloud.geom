#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices=6) out;

in vec2 gs_point_size[1];
in vec3 gs_color[1];
out vec3 fs_color;

void main(){
  // w and h are half-width, half-height
  float w = gs_point_size[0].x / 2;
  float h = gs_point_size[0].y / 2;
  fs_color = gs_color[0];

  gl_Position = gl_in[0].gl_Position + vec4(-w, -h, 0, 0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(w, -h, 0, 0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(-w, h, 0, 0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(w, -h, 0, 0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(w, h, 0, 0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(-w, h, 0, 0);
  EmitVertex();

  EndPrimitive();
}
