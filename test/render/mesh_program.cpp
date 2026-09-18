#include "datagui/render/program/mesh_program.hpp"
#include "datagui/asset/image.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/geometry/rot.hpp"
#include "datagui/render/window.hpp"
#include <GL/glew.h>
#include <cmath>
#include <cstdint>
#include <vector>

using namespace dgui;

namespace {

// A unit cube centred on the origin, with per-face normals.
void make_box(
    std::vector<Vec3>& positions,
    std::vector<Vec3>& normals,
    std::vector<unsigned int>& indices) {
  for (int axis = 0; axis < 3; axis++) {
    int a = (axis + 1) % 3;
    int b = (axis + 2) % 3;
    for (int dir = -1; dir <= 1; dir += 2) {
      unsigned int base = positions.size();
      Vec3 normal;
      normal(axis) = dir;
      for (int i = 0; i < 4; i++) {
        int sa = (i == 0 || i == 3) ? -1 : 1;
        int sb = (i < 2) ? -1 : 1;
        Vec3 position;
        position(axis) = 0.5 * dir;
        position(a) = 0.5 * sa;
        position(b) = 0.5 * sb;
        positions.push_back(position);
        normals.push_back(normal);
      }
      // Wind so the front face points along the normal.
      if (dir > 0) {
        indices.insert(
            indices.end(),
            {base, base + 1, base + 2, base, base + 2, base + 3});
      } else {
        indices.insert(
            indices.end(),
            {base, base + 2, base + 1, base, base + 3, base + 2});
      }
    }
  }
}

} // namespace

int main() {
  Window window;
  window.open("mesh_program", 800, 800);

  MeshProgram program;
  program.init();

  Camera3d camera;
  camera.direction.x = std::cos(M_PI / 4);
  camera.direction.z = -std::sin(M_PI / 4);
  camera.position.x = -6;
  camera.position.z = 5;
  camera.fov = Vec2::uniform(M_PI / 2);
  camera.clipping_min = 0.001;
  camera.clipping_max = 1000;

  // Fixed-color box
  Mesh fixed_mesh;
  {
    struct Vertex {
      Vec3 position;
      Vec3 normal;
    };
    std::vector<Vec3> positions, normals;
    std::vector<unsigned int> indices;
    make_box(positions, normals, indices);

    std::vector<Vertex> vertices(positions.size());
    for (std::size_t i = 0; i < positions.size(); i++) {
      vertices[i] = {positions[i], normals[i]};
    }
    fixed_mesh.load(
        indices.data(),
        indices.size(),
        vertices.data(),
        vertices.size(),
        offsetof(Vertex, position),
        offsetof(Vertex, normal),
        sizeof(Vertex));
  }

  // Per-vertex colored box
  Mesh colored_mesh;
  {
    struct Vertex {
      Vec3 position;
      Vec3 normal;
      Color color;
    };
    std::vector<Vec3> positions, normals;
    std::vector<unsigned int> indices;
    make_box(positions, normals, indices);

    std::vector<Vertex> vertices(positions.size());
    for (std::size_t i = 0; i < positions.size(); i++) {
      vertices[i].position = positions[i];
      vertices[i].normal = normals[i];
      vertices[i].color = Color(
          positions[i].x + 0.5,
          positions[i].y + 0.5,
          positions[i].z + 0.5);
    }
    colored_mesh.load_colored(
        indices.data(),
        indices.size(),
        vertices.data(),
        vertices.size(),
        offsetof(Vertex, position),
        offsetof(Vertex, normal),
        offsetof(Vertex, color),
        sizeof(Vertex));
  }

  // Textured torus
  Mesh textured_mesh;
  {
    struct Vertex {
      Vec3 position;
      Vec3 normal;
      Vec2 uv;
    };
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    std::size_t N = 50;
    float r1 = 1, r2 = 0.4;
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
        vertex.uv = Vec2{float(i % 2), float(j % 2)};
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

    struct Pixel {
      std::uint8_t r, g, b, a;
    };
    std::size_t width = 256;
    std::vector<Pixel> pixels(width * width);
    for (std::size_t i = 0; i < width; i++) {
      for (std::size_t j = 0; j < width; j++) {
        pixels[i * width + j] = ((i / 128 + j / 128) % 2 == 0)
                                    ? Pixel{255, 150, 150, 255}
                                    : Pixel{150, 150, 255, 255};
      }
    }

    Image texture;
    texture.load(width, width, pixels.data());
    textured_mesh.load_textured(
        indices.data(),
        indices.size(),
        vertices.data(),
        vertices.size(),
        offsetof(Vertex, position),
        offsetof(Vertex, normal),
        offsetof(Vertex, uv),
        sizeof(Vertex),
        texture);
  }

  while (window.running()) {
    window.render_begin();

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    const Mat4 view = camera.view_mat();
    const Mat4 projection = camera.projection_mat();

    program.bind();
    program.draw(
        view,
        projection,
        fixed_mesh,
        Mat4::transform(Vec3(0, 2, 0), Rot3()),
        Color::Red());
    program.draw(
        view,
        projection,
        colored_mesh,
        Mat4::transform(Vec3(0, -2, 0), Rot3()));
    program.draw(
        view,
        projection,
        textured_mesh,
        Mat4::transform(Vec3(0, 0, 3), Rot3()));
    // Translucent textured draw to exercise the opacity (mesh_color.a) path.
    program.draw(
        view,
        projection,
        textured_mesh,
        Mat4::transform(Vec3(3, 0, 0), Rot3()),
        Color(1.f, 1.f, 0.f, 0.8f));

    window.render_end();
    window.poll_events();
  }
}
