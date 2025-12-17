#include "datagui/visual/image_shader.hpp"
#include "datagui/geometry/rot.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <vector>

namespace datagui {

Image::~Image() {
  if (texture_ > 0) {
    glDeleteTextures(1, &texture_);
  }
}

Image::Image(Image&& other) {
  texture_ = other.texture_;
  other.texture_ = 0;
}

void Image::load(std::size_t width, std::size_t height, void* pixels) {
  if (texture_ == 0) {
    glGenTextures(1, &texture_);
  }
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      width,
      height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);
}

const static std::string vertex_shader = R"(
#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

uniform mat3 PV;
out vec2 fs_uv;

void main(){
  vec3 coords = PV * vec3(pos, 1);
  gl_Position = vec4(coords.xy / coords.z, 0, 1);
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

ImageShader::ImageShader() : program_id(0), uniform_PV(0), VAO(0), VBO(0) {}

ImageShader::~ImageShader() {
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

ImageShader::ImageShader(ImageShader&& other) {
  program_id = other.program_id;
  uniform_PV = other.uniform_PV;
  VAO = other.VAO;
  VBO = other.VBO;

  other.program_id = 0;
  other.VAO = 0;
  other.VBO = 0;
}

void ImageShader::init() {
  program_id = create_program(vertex_shader, fragment_shader);
  uniform_PV = glGetUniformLocation(program_id, "PV");

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

void ImageShader::queue_image(
    const Image& image,
    const Vec2& position,
    float angle,
    const Vec2& size) {
  if (!image.is_loaded()) {
    return;
  }

  Mat2 R = Rot2(angle).mat();
  Vec2 lower_left = position;
  Vec2 lower_right = position + R * Vec2(size.x, 0);
  Vec2 upper_left = position + R * Vec2(0, size.y);
  Vec2 upper_right = position + R * size;

  auto& command = commands.emplace_back();

  // Y flipped
  command.vertices = {
      Vertex{lower_left, Vec2(0, 1)},
      Vertex{lower_right, Vec2(1, 1)},
      Vertex{upper_left, Vec2(0, 0)},
      Vertex{lower_right, Vec2(1, 1)},
      Vertex{upper_right, Vec2(1, 0)},
      Vertex{upper_left, Vec2(0, 0)}};
  command.texture = image.texture();
}

void ImageShader::queue_masked_image(
    const Box2& mask,
    const Image& image,
    const Vec2& position,
    const Vec2& size) {
  if (!image.is_loaded()) {
    return;
  }

  Box2 region = intersection(mask, Box2(position, position + size));
  Box2 uv;
  uv.lower = (region.lower - position) / size;
  uv.upper = (region.upper - position) / size;

  // Y flipped
  uv.lower.y = 1 - uv.lower.y;
  uv.upper.y = 1 - uv.upper.y;

  auto& command = commands.emplace_back();
  command.vertices = {
      Vertex{region.lower_left(), uv.lower_left()},
      Vertex{region.lower_right(), uv.lower_right()},
      Vertex{region.upper_left(), uv.upper_left()},
      Vertex{region.lower_right(), uv.lower_right()},
      Vertex{region.upper_right(), uv.upper_right()},
      Vertex{region.upper_left(), uv.upper_left()}};
  command.texture = image.texture();
}

void ImageShader::queue_viewport(
    const Box2& mask,
    const Box2& box,
    int texture) {
  if (texture == 0) {
    return;
  }

  Box2 region = intersection(mask, box);
  Box2 uv;
  uv.lower = (region.lower - box.lower) / box.size();
  uv.upper = (region.upper - box.lower) / box.size();

  auto& command = commands.emplace_back();
  command.vertices = {
      Vertex{region.lower_left(), uv.lower_left()},
      Vertex{region.lower_right(), uv.lower_right()},
      Vertex{region.upper_left(), uv.upper_left()},
      Vertex{region.lower_right(), uv.lower_right()},
      Vertex{region.upper_right(), uv.upper_right()},
      Vertex{region.upper_left(), uv.upper_left()}};
  command.texture = texture;
}

void ImageShader::draw(const Box2& viewport, const Camera2d& camera) {
  if (commands.empty()) {
    return;
  }
  glViewport(
      viewport.lower.x,
      viewport.lower.y,
      viewport.upper.x - viewport.lower.x,
      viewport.upper.y - viewport.lower.y);

  Mat3 V = camera.view_mat();
  Mat3 P = camera.projection_mat();
  Mat3 PV = P * V;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  glUseProgram(program_id);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glUniformMatrix3fv(uniform_PV, 1, GL_FALSE, PV.data);

  for (const auto& command : commands) {
    glBindTexture(GL_TEXTURE_2D, command.texture);
    glBufferData(
        GL_ARRAY_BUFFER,
        command.vertices.size() * sizeof(Vertex),
        command.vertices.data(),
        GL_STATIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, command.vertices.size());
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void ImageShader::clear() {
  commands.clear();
}

} // namespace datagui
