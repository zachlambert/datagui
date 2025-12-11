#include "datagui/visual/texture_shader.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <vector>

namespace datagui {

const static std::string vertex_shader = R"(
#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

uniform vec2 viewport_size;
out vec2 fs_uv;

void main(){
  gl_Position = vec4(
    (pos.x - viewport_size.x / 2) / (viewport_size.x / 2),
    (pos.y - viewport_size.y / 2) / (viewport_size.y / 2),
    0,
    1
  );
  fs_uv = uv;
}
)";

const static std::string fragment_shader = R"(
#version 330 core

in vec2 fs_uv;

uniform sampler2D tex;
out vec4 color;

void main(){
  color = texture(tex, fs_uv);
}
)";

TextureShader::TextureShader() :
    program_id(0), uniform_viewport_size(0), VAO(0), VBO(0) {}

TextureShader::~TextureShader() {
  if (program_id > 0) {
    glDeleteProgram(program_id);
  }
  if (VAO > 0) {
    glDeleteVertexArrays(1, &VAO);
  }
  if (VBO > 0) {
    glDeleteBuffers(1, &VBO);
  }
}

TextureShader::ImageShader(TextureShader&& other) {
  program_id = other.program_id;
  uniform_viewport_size = other.uniform_viewport_size;
  VAO = other.VAO;
  VBO = other.VBO;

  other.program_id = 0;
  other.VAO = 0;
  other.VBO = 0;
}

void TextureShader::init() {
  program_id = create_program(vertex_shader, fragment_shader);
  uniform_viewport_size = glGetUniformLocation(program_id, "viewport_size");

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glVertexAttribPointer(
      0,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)offsetof(Vertex, pos));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(
      1,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)offsetof(Vertex, uv));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void TextureShader::queue_texture(const Box2& box, int texture) {
  commands.emplace_back();
  auto& command = commands.back();
  command.vertices = {
      Vertex{box.lower_left(), Vec2(0, 0)},
      Vertex{box.lower_right(), Vec2(1, 0)},
      Vertex{box.upper_left(), Vec2(0, 1)},
      Vertex{box.lower_right(), Vec2(1, 0)},
      Vertex{box.upper_right(), Vec2(1, 1)},
      Vertex{box.upper_left(), Vec2(0, 1)}};
  command.texture = texture;
}

void TextureShader::draw(const Vec2& viewport_size) {
  glUseProgram(program_id);
  glUniform2f(uniform_viewport_size, viewport_size.x, viewport_size.y);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  for (const auto& command : commands) {
    glBindTexture(GL_TEXTURE_2D, command.texture);
    glBufferData(
        GL_ARRAY_BUFFER,
        command.vertices.size() * sizeof(Vertex),
        command.vertices.data(),
        GL_STATIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, command.vertices.size());
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

} // namespace datagui
