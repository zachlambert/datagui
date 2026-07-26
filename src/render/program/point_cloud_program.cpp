#include "datagui/render/program/point_cloud_program.hpp"
#include "datagui/render/shaders.hpp"
#include <GL/glew.h>

namespace dgui {

void PointCloudProgram::init() {
  program_id = compile_program_vfg(
      shaders::point_cloud_vs,
      shaders::point_cloud_fs,
      shaders::point_cloud_gs);

  uniform_P = glGetUniformLocation(program_id, "P");
  uniform_V = glGetUniformLocation(program_id, "V");
  uniform_M = glGetUniformLocation(program_id, "M");
  uniform_point_size = glGetUniformLocation(program_id, "point_size");
}

void PointCloudProgram::bind() {
  glUseProgram(program_id);
}

void PointCloudProgram::draw(
    const PointCloud& point_cloud,
    const Mat4& model,
    float point_size,
    const Mat4& P,
    const Mat4& V,
    const Color& base_color) {

  if (!point_cloud.is_loaded()) {
    return;
  }

  glUniformMatrix4fv(uniform_P, 1, GL_FALSE, P.data);
  glUniformMatrix4fv(uniform_V, 1, GL_FALSE, V.data);
  glUniformMatrix4fv(uniform_M, 1, GL_FALSE, model.data);
  glUniform1f(uniform_point_size, point_size);
  // Default vertex color if buffer attribute not enabled
  glVertexAttrib3f(1, base_color.r, base_color.g, base_color.b);

  glBindVertexArray(point_cloud.data->VAO);
  glDrawArrays(GL_POINTS, 0, point_cloud.data->vertex_count);
  glBindVertexArray(0);
}

} // namespace dgui
