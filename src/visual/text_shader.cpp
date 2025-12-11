#include "datagui/visual/text_shader.hpp"
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
    (vertex_pos.x - viewport_size.x / 2) / (viewport_size.x / 2),
    (vertex_pos.y - viewport_size.y / 2) / (viewport_size.y / 2),
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

void TextShader::init(const std::shared_ptr<FontManager>& fm) {
  this->fm = fm;

  // Configure shader program and buffers

  program_id = create_program(vertex_shader, fragment_shader);
  uniform_viewport_size = glGetUniformLocation(program_id, "viewport_size");
  uniform_text_color = glGetUniformLocation(program_id, "text_color");

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  GLuint index = 0;

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)offsetof(Vertex, pos));
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)offsetof(Vertex, uv));
  glEnableVertexAttribArray(index);
  index++;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void TextShader::queue_text(
    const Vec2& origin,
    const std::string& text,
    Font font,
    int font_size,
    Color text_color,
    Length width,
    const Box2& mask) {
  queue_text(origin, 0.f, text, font, font_size, text_color, width, mask);
}

void TextShader::queue_text(
    const Vec2& origin,
    float angle,
    const std::string& text,
    Font font,
    int font_size,
    Color text_color,
    Length width) {
  queue_text(
      origin,
      angle,
      text,
      font,
      font_size,
      text_color,
      width,
      std::nullopt);
}

void TextShader::queue_text(
    const Vec2& origin,
    float angle,
    const std::string& text,
    Font font,
    int font_size,
    Color text_color,
    Length width,
    const std::optional<Box2>& mask) {

  // origin is at the top-left of the text box

  const auto& fs = fm->font_structure(font, font_size);

  std::size_t char_list_i = 0;
  while (char_list_i < char_lists.size()) {
    const auto& char_list = char_lists[char_list_i];
    if (char_list.font_texture == fs.font_texture &&
        char_list.font_color.equals(text_color)) {
      break;
    }
    char_list_i++;
  }
  if (char_list_i == char_lists.size()) {
    char_lists.emplace_back(fs.font_texture, text_color);
  }
  auto& vertices = char_lists[char_list_i].vertices;

  auto fixed_width = std::get_if<LengthFixed>(&width);
  Vec2 offset;
  offset.y -= fs.line_height;

  for (char c_char : text) {
    if (c_char == '\n') {
      offset.x = 0;
      offset.y -= fs.line_height;
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

    Vec2 char_offset = offset + c.offset + Vec2(0, fs.descender);
    offset.x += c.advance;
    Box2 uv = c.uv;

    if (mask) {
      assert(angle == 0);
      Vec2 pos = origin + char_offset;

      Box2 box(pos, pos + c.size);
      if (!intersects(*mask, box)) {
        // Not visible
        continue;
      }
      if (!contains(*mask, box)) {
        // Partially obscured -> alter box and uv
        Box2 new_box = intersection(*mask, box);
        Box2 new_uv;
        new_uv.lower.x = uv.lower.x + uv.size().x *
                                          (new_box.lower.x - box.lower.x) /
                                          box.size().x;
        new_uv.lower.y = uv.lower.y + uv.size().y *
                                          (new_box.lower.y - box.lower.y) /
                                          box.size().y;
        new_uv.upper.x = uv.lower.x + uv.size().x *
                                          (new_box.upper.x - box.lower.x) /
                                          box.size().x;
        new_uv.upper.y = uv.lower.y + uv.size().y *
                                          (new_box.upper.y - box.lower.y) /
                                          box.size().y;
        box = new_box;
        uv = new_uv;
      }
      vertices.push_back(Vertex{box.lower_left(), uv.lower_left()});
      vertices.push_back(Vertex{box.lower_right(), uv.lower_right()});
      vertices.push_back(Vertex{box.upper_left(), uv.upper_left()});
      vertices.push_back(Vertex{box.lower_right(), uv.lower_right()});
      vertices.push_back(Vertex{box.upper_right(), uv.upper_right()});
      vertices.push_back(Vertex{box.upper_left(), uv.upper_left()});

    } else {
      Mat2 rot = Rot2(angle).mat();
      Vec2 lower_left = origin + rot * char_offset;
      Vec2 lower_right = origin + rot * (char_offset + Vec2(c.size.x, 0));
      Vec2 upper_left = origin + rot * (char_offset + Vec2(0, c.size.y));
      Vec2 upper_right = origin + rot * (char_offset + c.size);

      vertices.push_back(Vertex{lower_left, uv.lower_left()});
      vertices.push_back(Vertex{lower_right, uv.lower_right()});
      vertices.push_back(Vertex{upper_left, uv.upper_left()});
      vertices.push_back(Vertex{lower_right, uv.lower_right()});
      vertices.push_back(Vertex{upper_right, uv.upper_right()});
      vertices.push_back(Vertex{upper_left, uv.upper_left()});
    }
  }
}

void TextShader::draw(const Vec2& viewport_size) {
  if (char_lists.empty()) {
    return;
  }

  glUseProgram(program_id);
  glBindVertexArray(VAO);
  glUniform2f(uniform_viewport_size, viewport_size.x, viewport_size.y);

  for (const auto& char_list : char_lists) {
    if (char_list.vertices.empty()) {
      continue;
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        char_list.vertices.size() * sizeof(Vertex),
        char_list.vertices.data(),
        GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUniform4f(
        uniform_text_color,
        char_list.font_color.r,
        char_list.font_color.g,
        char_list.font_color.b,
        char_list.font_color.a);

    glBindTexture(GL_TEXTURE_2D, char_list.font_texture);
    glDrawArrays(GL_TRIANGLES, 0, char_list.vertices.size());
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  glBindVertexArray(0);
  glUseProgram(0);

  char_lists.clear();
}

} // namespace datagui
