#include "datagui/render/program/point_cloud_program.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/geometry/rot.hpp"
#include "datagui/visual/window.hpp"
#include <GL/glew.h>
#include <cmath>
#include <vector>

using namespace dgui;

int main() {
  Window window;
  window.open("point_cloud_program", 800, 800);

  PointCloudProgram program;
  program.init();

  Camera3d camera;
  camera.direction.x = std::cos(M_PI / 4);
  camera.direction.z = -std::sin(M_PI / 4);
  camera.position.x = -3;
  camera.position.z = 2;
  camera.fov = Vec2::uniform(M_PI / 2);
  camera.clipping_min = 0.001;
  camera.clipping_max = 1000;

  // Per-point colored cloud: a lattice cube coloured by position.
  PointCloud colored_cloud;
  {
    struct Point {
      Vec3 position;
      Vec3 color;
    };
    std::vector<Point> points;
    for (float x = -0.5; x <= 0.5; x += 0.1) {
      for (float y = -0.5; y <= 0.5; y += 0.1) {
        for (float z = -0.5; z <= 0.5; z += 0.1) {
          Point point;
          point.position = Vec3(x, y, z);
          point.color = Vec3(x + 0.5, y + 0.5, z + 0.5);
          points.push_back(point);
        }
      }
    }
    colored_cloud.load_colored(
        points.data(),
        points.size(),
        offsetof(Point, position),
        offsetof(Point, color),
        sizeof(Point));
  }

  // Single-color cloud: same lattice, colour supplied per draw call.
  PointCloud fixed_cloud;
  {
    std::vector<Vec3> points;
    for (float x = -0.5; x <= 0.5; x += 0.1) {
      for (float y = -0.5; y <= 0.5; y += 0.1) {
        for (float z = -0.5; z <= 0.5; z += 0.1) {
          points.push_back(Vec3(x, y, z));
        }
      }
    }
    fixed_cloud.load(points.data(), points.size(), 0, sizeof(Vec3));
  }

  while (window.running()) {
    window.render_begin();

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    Mat4 V = camera.view_mat();
    Mat4 P = camera.projection_mat();

    program.bind();
    program.draw(
        colored_cloud, Mat4::transform(Vec3(0, 2, 0), Rot3()), 0.05, P, V);
    program.draw(
        fixed_cloud,
        Mat4::transform(Vec3(0, -2, 0), Rot3()),
        0.05,
        P,
        V,
        Color(1, 0.6, 0.2));

    window.render_end();
    window.poll_events();
  }
}
