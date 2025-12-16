#include "datagui/visual/text_2d_shader.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <assert.h>
#include <string>

namespace datagui {

const static std::string vertex_shader = R"(
#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

uniform mat3 PV;
out vec2 fs_uv;

void main(){
  gl_Position = vec4((PV * vec3(position, 1)).xy, 0, 1);
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

void Text2dShader::init(const std::shared_ptr<FontManager>& fm) {
  this->fm = fm;

  // Configure shader program and buffers

  program_id = create_program(vertex_shader, fragment_shader);
  uniform_PV = glGetUniformLocation(program_id, "PV");
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

void Text2dShader::queue_masked_text(
    const Box2& mask,
    const Vec2& origin,
    const std::string& text,
    Font font,
    int font_size,
    Color text_color,
    Length width) {

  auto characters = fm->text_characters(text, font, font_size, width);
  auto& vertices = get_vertices(font, font_size, text_color);

  // Mutable reference to [box, uv] so they can be modified if necessary
  for (auto& [box, uv] : characters) {
    box.lower += origin;
    box.upper += origin;
    if (!intersects(mask, box)) {
      continue;
    }
    if (!contains(mask, box)) {
      // Partially obscured -> alter box and uv
      Box2 new_box = intersection(mask, box);
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
  }
}

void Text2dShader::queue_text(
    const Vec2& origin,
    float angle,
    const std::string& text,
    Font font,
    int font_size,
    Color text_color,
    Length width) {

  auto characters = fm->text_characters(text, font, font_size, width);
  auto& vertices = get_vertices(font, font_size, text_color);

  for (const auto& [box, uv] : characters) {
    Mat2 rot = Rot2(angle).mat();
    Vec2 lower_left = origin + rot * box.lower_left();
    Vec2 lower_right = origin + rot * box.lower_right();
    Vec2 upper_left = origin + rot * box.upper_left();
    Vec2 upper_right = origin + rot * box.upper_right();

    vertices.push_back(Vertex{lower_left, uv.lower_left()});
    vertices.push_back(Vertex{lower_right, uv.lower_right()});
    vertices.push_back(Vertex{upper_left, uv.upper_left()});
    vertices.push_back(Vertex{lower_right, uv.lower_right()});
    vertices.push_back(Vertex{upper_right, uv.upper_right()});
    vertices.push_back(Vertex{upper_left, uv.upper_left()});
  }
}

std::vector<Text2dShader::Vertex>& Text2dShader::get_vertices(
    Font font,
    int font_size,
    const Color& color) {
  const auto& fs = fm->font_structure(font, font_size);

  std::size_t char_list_i = 0;
  while (char_list_i < char_lists.size()) {
    const auto& char_list = char_lists[char_list_i];
    if (char_list.font_texture == fs.font_texture &&
        char_list.font_color.equals(color)) {
      break;
    }
    char_list_i++;
  }
  if (char_list_i == char_lists.size()) {
    return char_lists.emplace_back(fs.font_texture, color).vertices;
  } else {
    return char_lists[char_list_i].vertices;
  }
}

void Text2dShader::draw(const Box2& viewport, const Camera2d& camera) {
  if (char_lists.empty()) {
    return;
  }
  glViewport(
      viewport.lower.x,
      viewport.lower.y,
      viewport.upper.x,
      viewport.upper.y);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  Mat3 V = camera.view_mat();
  Mat3 P = camera.projection_mat();
  Mat3 PV = P * V;

  glUseProgram(program_id);
  glBindVertexArray(VAO);
  glUniformMatrix3fv(uniform_PV, 1, GL_FALSE, PV.data);

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
}

void Text2dShader::clear() {
  char_lists.clear();
}

} // namespace datagui
