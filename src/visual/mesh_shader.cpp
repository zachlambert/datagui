#include "datagui/visual/mesh_shader.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>

namespace datagui {

const static std::string vertex_shader = R"(
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 fs_normal_cs;
out vec3 fs_normal_ws;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main(){
  mat4 VM = V * M;
  mat4 PVM = P * VM;
  gl_Position = PVM * vec4(position, 1);
  fs_normal_cs = normalize((VM * vec4(normal, 0)).xyz);
  fs_normal_ws = normalize((M * vec4(normal, 0)).xyz);
}
)";

const static std::string fragment_shader = R"(
#version 330 core

in vec3 fs_normal_cs;
in vec3 fs_normal_ws;
out vec4 color;

uniform vec4 mesh_color;

void main(){
  const vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
  // https://godotshaders.com/shader/transparency-dither/
	if (mesh_color.a < 0.001 ||
      mesh_color.a < fract(magic.z * fract(dot(gl_FragCoord.xy, magic.xy)))) {
		discard;
	}

  float Ka = 0.2;
  float Kc = 0.5;
  float Kw = 0.3;
  float camera_light = clamp(dot(fs_normal_cs, vec3(0, 0, 1)), 0, 1);
  float world_light = clamp(dot(fs_normal_ws, vec3(0, 0, 1)), 0, 1);
  color = mesh_color * min(Ka + Kc * camera_light + Kw * world_light, 1);
}
)";

Mesh::~Mesh() {
  if (VAO > 0) {
    glDeleteVertexArrays(1, &VAO);
  }
  if (VBO > 0) {
    glDeleteBuffers(1, &VBO);
  }
  if (EBO > 0) {
    glDeleteBuffers(1, &EBO);
  }
}

Mesh::Mesh(Mesh&& other) {
  VAO = other.VAO;
  VBO = other.VBO;
  EBO = other.VBO;
  index_count = other.index_count;
  other.VAO = 0;
  other.VBO = 0;
  other.EBO = 0;
}

void Mesh::init() {
  assert(!initialized);
  assert(VAO == 0);
  assert(VBO == 0);
  assert(EBO == 0);

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

  GLuint index = 0;
  glVertexAttribPointer(
      index,
      3,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)(offsetof(Vertex, position)));
  glEnableVertexAttribArray(index);
  index++;

  glVertexAttribPointer(
      index,
      3,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)(offsetof(Vertex, normal)));
  glEnableVertexAttribArray(index);
  index++;

  glBindVertexArray(0);

  initialized = true;
}

void Mesh::load_vertices(
    void* vertices,
    std::size_t num_vertices,
    std::size_t positions_offset,
    std::size_t normals_offset,
    std::size_t stride) {

  if (!initialized) {
    init();
  }

  std::vector<Vertex> gl_vertices(num_vertices);
  for (std::size_t i = 0; i < num_vertices; i++) {
    gl_vertices[i].position =
        *(Vec3*)((std::uint8_t*)vertices + i * stride + positions_offset);
    gl_vertices[i].normal =
        *(Vec3*)((std::uint8_t*)vertices + i * stride + normals_offset);
  }

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      gl_vertices.size() * sizeof(Vertex),
      gl_vertices.data(),
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::load_indices(unsigned int* const indices, std::size_t num_indices) {
  if (!initialized) {
    init();
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      num_indices * sizeof(unsigned int),
      indices,
      GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  index_count = num_indices;
}

void MeshShader::init() {
  program_id = create_program(vertex_shader, fragment_shader);

  uniform_P = glGetUniformLocation(program_id, "P");
  uniform_V = glGetUniformLocation(program_id, "V");
  uniform_M = glGetUniformLocation(program_id, "M");
  uniform_mesh_color = glGetUniformLocation(program_id, "mesh_color");
}

void MeshShader::queue_mesh(
    const Mesh& mesh,
    const Vec3& position,
    const Rot3& orientation,
    const Color& color) {
  auto& command = commands.emplace_back();

  command.VAO = mesh.VAO;
  command.index_count = mesh.index_count;

  command.model_mat = Mat4();
  command.model_mat(3, 3) = 1;
  for (std::size_t i = 0; i < 3; i++) {
    for (std::size_t j = 0; j < 3; j++) {
      command.model_mat(i, j) = orientation.mat()(i, j);
    }
  }
  for (std::size_t i = 0; i < 3; i++) {
    command.model_mat(i, 3) = position(i);
  }

  command.color = color;
}

void MeshShader::draw(const Box2& viewport, const Camera3d& camera) {
  if (commands.empty()) {
    return;
  }
  glViewport(
      viewport.lower.x,
      viewport.lower.y,
      viewport.upper.x - viewport.lower.x,
      viewport.upper.y - viewport.lower.y);

  Mat4 V = camera.view_mat();
  Mat4 P = camera.projection_mat();

  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glUseProgram(program_id);
  glUniformMatrix4fv(uniform_V, 1, GL_FALSE, V.data);
  glUniformMatrix4fv(uniform_P, 1, GL_FALSE, P.data);

  for (const auto& command : commands) {
    glUniformMatrix4fv(uniform_M, 1, GL_FALSE, command.model_mat.data);
    glUniform4fv(uniform_mesh_color, 1, command.color.data);

    glBindVertexArray(command.VAO);
    glDrawElements(
        GL_TRIANGLES,
        command.index_count,
        GL_UNSIGNED_INT,
        (void*)0);
  }

  glBindVertexArray(0);
}

void MeshShader::clear() {
  commands.clear();
}

} // namespace datagui
