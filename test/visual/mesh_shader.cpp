#include <datagui/visual/mesh_shader.hpp>
#include <datagui/visual/window.hpp>

int main() {
  using namespace datagui;

  Window window;
  MeshShader mesh_shader;
  mesh_shader.init();

  Camera3d camera;
  camera.direction.x = std::cos(M_PI / 4);
  camera.direction.y = 0;
  camera.direction.z = -std::sin(M_PI / 4);
  camera.position.x = -2;
  camera.position.z = 2;
  camera.fov_degrees = 90;
  camera.clipping_min = 0.001;
  camera.clipping_max = 1000;
  camera.size = window.size();

  struct Vertex {
    Vec3 position;
    Vec3 normal;
  };
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  std::size_t N = 50;
  float r1 = 1;
  float r2 = 0.5;
  for (std::size_t i = 0; i < N; i++) {
    float theta = 2 * i * M_PIf / N;
    Vec3 n1(std::cos(theta), std::sin(theta), 0);
    for (std::size_t j = 0; j < N; j++) {
      float phi = 2 * j * M_PIf / N;
      Vec3 n2(
          std::cos(theta) * std::cos(phi),
          std::sin(theta) * std::cos(phi),
          std::sin(phi));
      Vertex vertex;
      vertex.position = n1 * r1 + n2 * r2;
      vertex.normal = n2;
      vertices.push_back(vertex);
    }
  }
  for (std::size_t i = 0; i < N; i++) {
    for (std::size_t j = 0; j < N; j++) {
      indices.push_back(i * N + j);
      indices.push_back(((i + 1) % N) * N + j);
      indices.push_back(i * N + (j + 1) % N);
      indices.push_back(i * N + (j + 1) % N);
      indices.push_back(((i + 1) % N) * N + j);
      indices.push_back(((i + 1) % N) * N + (j + 1) % N);
    }
  }

  Mesh mesh;
  mesh.load_vertices(
      vertices.data(),
      vertices.size(),
      offsetof(Vertex, position),
      offsetof(Vertex, normal),
      sizeof(Vertex));
  mesh.load_indices(indices.data(), indices.size());

  while (window.running()) {
    window.render_begin();

    mesh_shader.queue_mesh(mesh, Vec3(), Rot3(), Color::Red());
    mesh_shader.draw(Box2(Vec2(), window.size()), camera);
    mesh_shader.clear();

    window.render_end();
    window.poll_events();
  }
}
