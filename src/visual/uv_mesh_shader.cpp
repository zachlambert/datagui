#include "datagui/visual/uv_mesh_shader.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <cstring>

namespace datagui {

const static std::string vertex_shader = R"(
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out vec3 fs_normal_cs;
out vec3 fs_normal_ws;
out vec2 fs_uv;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main(){
  mat4 VM = V * M;
  mat4 PVM = P * VM;
  gl_Position = PVM * vec4(position, 1);
  fs_normal_cs = normalize((VM * vec4(normal, 0)).xyz);
  fs_normal_ws = normalize((M * vec4(normal, 0)).xyz);
  fs_uv = uv;
}
)";

const static std::string fragment_shader = R"(
#version 330 core

in vec3 fs_normal_cs;
in vec3 fs_normal_ws;
in vec2 fs_uv;
out vec4 color;

uniform float opacity;
uniform sampler2D mesh_texture;

void main(){
  vec4 frag_color = texture(mesh_texture, fs_uv);
  frag_color.a *= opacity;

  const vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
  // https://godotshaders.com/shader/transparency-dither/
	if (frag_color.a < 0.001 ||
      frag_color.a < fract(magic.z * fract(dot(gl_FragCoord.xy, magic.xy)))) {
		discard;
	}

  float Ka = 0.2;
  float Kc = 0.5;
  float Kw = 0.3;
  float camera_light = clamp(dot(fs_normal_cs, vec3(0, 0, 1)), 0, 1);
  float world_light = clamp(dot(fs_normal_ws, vec3(0, 0, 1)), 0, 1);
  color = frag_color * min(Ka + Kc * camera_light + Kw * world_light, 1);
}
)";

void UvMeshShader::init() {
  program_id = create_program(vertex_shader, fragment_shader);

  uniform_P = glGetUniformLocation(program_id, "P");
  uniform_V = glGetUniformLocation(program_id, "V");
  uniform_M = glGetUniformLocation(program_id, "M");
  uniform_opacity = glGetUniformLocation(program_id, "opacity");
}

void UvMeshShader::queue_mesh(
    const UvMesh& uv_mesh,
    const Vec3& position,
    const Rot3& orientation,
    float opacity) {
  auto& command = commands.emplace_back();
  command.uv_mesh = uv_mesh;
  command.model_mat = Mat4::Transform(position, orientation);
  command.opacity = opacity;
}

void UvMeshShader::draw(const Box2& viewport, const Camera3d& camera) {
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
    glUniform1f(uniform_opacity, command.opacity);

    glBindVertexArray(command.uv_mesh.data->VAO);
    glBindTexture(GL_TEXTURE_2D, command.uv_mesh.data->texture);

    glDrawElements(
        GL_TRIANGLES,
        command.uv_mesh.data->index_count,
        GL_UNSIGNED_INT,
        (void*)0);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
}

void UvMeshShader::clear() {
  commands.clear();
}

} // namespace datagui
