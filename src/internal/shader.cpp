#include "datagui/internal/shader.hpp"

#include <GL/glew.h>
#include <fstream>
#include <iostream>
#include <sstream>

namespace datagui {

bool load_file_into_string(const std::string& file_path, std::string& string) {
  std::ifstream file_stream(file_path, std::ios::in);
  if (file_stream.is_open()) {
    std::stringstream sstream;
    sstream << file_stream.rdbuf();
    string = sstream.str();
    file_stream.close();
    return true;
  } else {
    std::cerr << "Failed to open file " << file_path << std::endl;
    return false;
  }
}

void compile_and_check_shader(
    const std::string& code,
    const unsigned int shader_id) {
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
    std::cerr << vertex_shader_error_message << std::endl;
  }
}

unsigned int create_program(
    const std::string& vs_code,
    const std::string& fs_code) {
  // Create shaders
  unsigned int vertex_program_id = glCreateShader(GL_VERTEX_SHADER);
  unsigned int fragment_program_id = glCreateShader(GL_FRAGMENT_SHADER);

  compile_and_check_shader(vs_code, vertex_program_id);

  compile_and_check_shader(fs_code, fragment_program_id);

  // Link the program
  GLuint program_id = glCreateProgram();
  glAttachShader(program_id, vertex_program_id);
  glAttachShader(program_id, fragment_program_id);
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

  glDetachShader(program_id, vertex_program_id);
  glDetachShader(program_id, fragment_program_id);

  glDeleteShader(vertex_program_id);
  glDeleteShader(fragment_program_id);

  return program_id;
}

} // namespace datagui
