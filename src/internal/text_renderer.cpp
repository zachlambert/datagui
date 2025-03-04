#include "datagui/internal/text_renderer.hpp"
#include "datagui/internal/shader.hpp"
#include <GL/glew.h>
#include <string>

namespace datagui {

const static std::string vertex_shader = R"(
#version 330 core

// Input vertex data: position and normal
layout(location = 0) in vec2 vertex_pos;
layout(location = 1) in vec2 uv;

uniform vec2 viewport_size;
out vec2 fs_uv;

void main(){
  gl_Position = vec4(
    -1.f + 2 * vertex_pos.x / viewport_size.x,
    1.f - 2 * vertex_pos.y / viewport_size.y,
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
uniform vec4 text_color;

out vec4 color;

void main(){
  color = vec4(text_color.xyz, texture(tex, fs_uv).x * text_color.a);
}
)";

void TextRenderer::init() {
  // Configure shader program and buffers

  gl_data.program_id = create_program(vertex_shader, fragment_shader);
  gl_data.uniform_viewport_size =
      glGetUniformLocation(gl_data.program_id, "viewport_size");
  gl_data.uniform_text_color =
      glGetUniformLocation(gl_data.program_id, "text_color");

  glGenVertexArrays(1, &gl_data.VAO);
  glGenBuffers(1, &gl_data.VBO);

  glBindVertexArray(gl_data.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, gl_data.VBO);

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

void TextRenderer::queue_text(
    const FontStructure& font,
    const Color& font_color,
    const std::string& text,
    float max_width,
    const Vecf& origin) {
  std::size_t command_i = 0;
  while (command_i < commands.size()) {
    const auto& command = commands[command_i];
    if (command.font_texture == font.font_texture &&
        command.font_color.equals(font_color)) {
      break;
    }
    command_i++;
  }
  if (command_i == commands.size()) {
    commands.emplace_back(font.font_texture, font_color);
  }
  auto& vertices = commands[command_i].vertices;

  bool has_max_width = max_width > 0;
  Vecf offset = Vecf::Zero();
  offset.y += font.line_height;

  for (char c_char : text) {
    if (!font.char_valid(c_char)) {
      continue;
    }
    const auto& c = font.get(c_char);

    if (has_max_width && offset.x + c.advance > max_width) {
      offset.x = 0;
      offset.y += font.line_height;
    }

    Vecf pos = origin + offset;
    pos.x += c.offset.x;
    pos.y += (-font.descender - (c.offset.y + c.size.y));

    Boxf box(pos, pos + c.size);
    const Boxf& uv = c.uv;

    vertices.push_back(Vertex{box.bottom_left(), uv.top_left()});
    vertices.push_back(Vertex{box.bottom_right(), uv.top_right()});
    vertices.push_back(Vertex{box.top_left(), uv.bottom_left()});
    vertices.push_back(Vertex{box.bottom_right(), uv.top_right()});
    vertices.push_back(Vertex{box.top_right(), uv.bottom_right()});
    vertices.push_back(Vertex{box.top_left(), uv.bottom_left()});

    offset.x += c.advance;
  }
}

void TextRenderer::render(const Vecf& viewport_size) {
  glUseProgram(gl_data.program_id);
  glBindVertexArray(gl_data.VAO);
  glUniform2f(gl_data.uniform_viewport_size, viewport_size.x, viewport_size.y);
  for (const auto& command : commands) {
    glBindBuffer(GL_ARRAY_BUFFER, gl_data.VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        command.vertices.size() * sizeof(Vertex),
        command.vertices.data(),
        GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUniform4f(
        gl_data.uniform_text_color,
        command.font_color.r,
        command.font_color.g,
        command.font_color.b,
        command.font_color.a);

    glBindTexture(GL_TEXTURE_2D, command.font_texture);
    glDrawArrays(GL_TRIANGLES, 0, command.vertices.size());
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  glBindVertexArray(0);
  glUseProgram(0);

  commands.clear();
}

} // namespace datagui
