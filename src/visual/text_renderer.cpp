#include "datagui/visual/text_renderer.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <assert.h>
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

void TextRenderer::init(std::shared_ptr<FontManager> fm) {
  this->fm = fm;

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
    const Vec2& origin,
    const std::string& text,
    Font font,
    int font_size,
    Color text_color,
    Length width,
    const Box2& mask) {

  const auto& fs = fm->font_structure(font, font_size);

  std::size_t command_i = 0;
  while (command_i < commands.size()) {
    const auto& command = commands[command_i];
    if (command.font_texture == fs.font_texture &&
        command.font_color.equals(text_color)) {
      break;
    }
    command_i++;
  }
  if (command_i == commands.size()) {
    commands.emplace_back(fs.font_texture, text_color);
  }
  auto& vertices = commands[command_i].vertices;

  auto fixed_width = std::get_if<LengthFixed>(&width);
  Vec2 offset;
  offset.y += fs.line_height;

  for (char c_char : text) {
    if (c_char == '\n') {
      offset.x = 0;
      offset.y += fs.line_height;
      continue;
    }
    if (!fs.char_valid(c_char)) {
      continue;
    }
    const auto& c = fs.get(c_char);

    if (fixed_width && offset.x + c.advance > fixed_width->value) {
      offset.x = 0;
      offset.y += fs.line_height;
    }

    Vec2 pos = origin + offset;
    pos.x += c.offset.x;
    pos.y += (-fs.descender - (c.offset.y + c.size.y));
    offset.x += c.advance;

    Box2 box(pos, pos + c.size);
    Box2 uv = c.uv;

    if (!intersects(mask, box)) {
      // Not visible
      continue;
    }
    if (!contains(mask, box)) {
      // Partially obscured -> alter box and uv
      Box2 new_box = intersection(mask, box);
      Box2 new_uv;
      new_uv.lower.x = uv.lower.x + uv.size().x *
                                        (new_box.lower.x - box.lower.x) /
                                        box.size().x;
      new_uv.lower.y = uv.lower.y + uv.size().x *
                                        (new_box.lower.x - box.lower.x) /
                                        box.size().x;
      new_uv.upper.x = uv.lower.x + uv.size().x *
                                        (new_box.upper.x - box.lower.x) /
                                        box.size().x;
      new_uv.upper.y = uv.lower.y + uv.size().y *
                                        (new_box.upper.y - box.lower.y) /
                                        box.size().y;
      box = new_box;
      uv = new_uv;
    }

    vertices.push_back(Vertex{box.lower_left(), uv.upper_left()});
    vertices.push_back(Vertex{box.lower_right(), uv.upper_right()});
    vertices.push_back(Vertex{box.upper_left(), uv.lower_left()});
    vertices.push_back(Vertex{box.lower_right(), uv.upper_right()});
    vertices.push_back(Vertex{box.upper_right(), uv.lower_right()});
    vertices.push_back(Vertex{box.upper_left(), uv.lower_left()});
  }
}

void TextRenderer::render(const Vec2& viewport_size) {
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
