#include "datagui/visual/shader_utils.hpp"

#include <GL/glew.h>
#include <iostream>
#include <vector>

namespace datagui {

unsigned int load_shader(const std::string& code, unsigned int shader_type) {
  unsigned int shader_id = glCreateShader(shader_type);

  GLint result = GL_FALSE;
  int info_log_length;

  // Compile shader
  char const* vertex_source_pointer = code.c_str();
  glShaderSource(shader_id, 1, &vertex_source_pointer, NULL);
  glCompileShader(shader_id);

  // Check shader
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
  glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
  if (info_log_length > 0) {
    char vertex_shader_error_message[info_log_length + 1];
    glGetShaderInfoLog(
        shader_id,
        info_log_length,
        NULL,
        vertex_shader_error_message);
    std::cerr << "================\n" << code << "\n--------------\n";
    std::cerr << vertex_shader_error_message << std::endl;
  }
  return shader_id;
}

static unsigned int create_program(
    const std::vector<unsigned int>& shader_ids) {

  GLuint program_id = glCreateProgram();
  for (unsigned int shader_id : shader_ids) {
    glAttachShader(program_id, shader_id);
  }
  glLinkProgram(program_id);

  // Check the program
  GLint result = GL_FALSE;
  int info_log_length;

  glGetProgramiv(program_id, GL_LINK_STATUS, &result);
  glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
  if (info_log_length > 0) {
    char program_error_message[info_log_length + 1];
    glGetProgramInfoLog(
        program_id,
        info_log_length,
        NULL,
        program_error_message);
    std::cerr << program_error_message << std::endl;
  }

  for (unsigned int shader_id : shader_ids) {
    glDetachShader(program_id, shader_id);
    glDeleteShader(shader_id);
  }

  return program_id;
}

unsigned int create_program(
    const std::string& vs_code,
    const std::string& fs_code) {
  return create_program(
      {load_shader(vs_code, GL_VERTEX_SHADER),
       load_shader(fs_code, GL_FRAGMENT_SHADER)});
}

unsigned int create_program_gs(
    const std::string& vs_code,
    const std::string& fs_code,
    const std::string& gs_code) {
  return create_program(
      {load_shader(vs_code, GL_VERTEX_SHADER),
       load_shader(fs_code, GL_FRAGMENT_SHADER),
       load_shader(gs_code, GL_GEOMETRY_SHADER)});
}

} // namespace datagui
