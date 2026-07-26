#include "datagui/render/program/uv_mesh_program.hpp"
#include "datagui/render/shaders.hpp"
#include <GL/glew.h>

namespace dgui {

void UvMeshProgram::init() {
  program_id = compile_program_vf(shaders::uv_mesh_vs, shaders::uv_mesh_fs);

  uniform_P = glGetUniformLocation(program_id, "P");
  uniform_V = glGetUniformLocation(program_id, "V");
  uniform_M = glGetUniformLocation(program_id, "M");
  uniform_opacity = glGetUniformLocation(program_id, "opacity");
}

void UvMeshProgram::bind() {
  glUseProgram(program_id);
}

void UvMeshProgram::draw(
    const UvMesh& uv_mesh,
    const Mat4& model,
    float opacity,
    const Mat4& P,
    const Mat4& V) {
  if (!uv_mesh.is_loaded()) {
    return;
  }

  glUniformMatrix4fv(uniform_P, 1, GL_FALSE, P.data);
  glUniformMatrix4fv(uniform_V, 1, GL_FALSE, V.data);
  glUniformMatrix4fv(uniform_M, 1, GL_FALSE, model.data);
  glUniform1f(uniform_opacity, opacity);

  glBindVertexArray(uv_mesh.data->VAO);
  glBindTexture(GL_TEXTURE_2D, uv_mesh.data->texture);
  glDrawElements(
      GL_TRIANGLES,
      uv_mesh.data->index_count,
      GL_UNSIGNED_INT,
      (void*)0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
}

} // namespace dgui
