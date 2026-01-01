#include "datagui/visual/point_cloud_shader.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>

namespace datagui {

const static std::string vertex_shader = R"(
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
)";

const static std::string geometry_shader = R"(
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
)";

const static std::string fragment_shader = R"(
#version 330 core

in vec3 fs_color;
out vec4 color;

void main(){
  color = vec4(fs_color, 1);
}
)";

void PointCloudShader::init() {
  program_id =
      create_program_gs(vertex_shader, fragment_shader, geometry_shader);

  uniform_P = glGetUniformLocation(program_id, "P");
  uniform_V = glGetUniformLocation(program_id, "V");
  uniform_M = glGetUniformLocation(program_id, "M");
  uniform_point_size = glGetUniformLocation(program_id, "point_size");
}

void PointCloudShader::queue_point_cloud(
    const PointCloud& point_cloud,
    const Vec3& position,
    const Rot3& orientation,
    float point_size) {
  auto& command = commands.emplace_back();
  command.point_cloud = point_cloud;
  command.model_mat = Mat4::Transform(position, orientation);
  command.point_size = point_size;
}

void PointCloudShader::draw(const Box2& viewport, const Camera3d& camera) {
  if (commands.empty()) {
    return;
  }
  glViewport(
      viewport.lower.x,
      viewport.lower.y,
      viewport.upper.x - viewport.lower.x,
      viewport.upper.y - viewport.lower.y);

  Mat4 V = camera.view_mat();
  Mat4 P = camera.projection_mat();

  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glUseProgram(program_id);
  glUniformMatrix4fv(uniform_V, 1, GL_FALSE, V.data);
  glUniformMatrix4fv(uniform_P, 1, GL_FALSE, P.data);

  for (const auto& command : commands) {
    glUniformMatrix4fv(uniform_M, 1, GL_FALSE, command.model_mat.data);
    glUniform1f(uniform_point_size, command.point_size);
    glBindVertexArray(command.point_cloud.data->VAO);
    glDrawArrays(GL_POINTS, 0, command.point_cloud.data->vertex_count);
  }

  glBindVertexArray(0);
}

void PointCloudShader::clear() {
  commands.clear();
}

} // namespace datagui
