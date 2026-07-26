#include "datagui/render/program/mesh_program.hpp"
#include "datagui/render/shaders.hpp"
#include <GL/glew.h>

namespace dgui {

void MeshProgram::init() {
  program_id = compile_program_vf(shaders::mesh_vs, shaders::mesh_fs);

  uniform_P = glGetUniformLocation(program_id, "P");
  uniform_V = glGetUniformLocation(program_id, "V");
  uniform_M = glGetUniformLocation(program_id, "M");
  uniform_mesh_color = glGetUniformLocation(program_id, "mesh_color");
}

void MeshProgram::bind() {
  glUseProgram(program_id);
}

void MeshProgram::draw(
    const Mesh& mesh,
    const Mat4& model,
    const Color& color,
    const Mat4& P,
    const Mat4& V) {
  if (!mesh.is_loaded()) {
    return;
  }

  glUniformMatrix4fv(uniform_P, 1, GL_FALSE, P.data);
  glUniformMatrix4fv(uniform_V, 1, GL_FALSE, V.data);
  glUniformMatrix4fv(uniform_M, 1, GL_FALSE, model.data);
  glUniform4fv(uniform_mesh_color, 1, color.data);

  glBindVertexArray(mesh.data->VAO);
  glDrawElements(GL_TRIANGLES, mesh.data->index_count, GL_UNSIGNED_INT, (void*)0);
  glBindVertexArray(0);
}

} // namespace dgui
