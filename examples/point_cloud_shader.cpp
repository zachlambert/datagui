#include <datagui/visual/point_cloud_shader.hpp>
#include <datagui/visual/window.hpp>

int main() {
  using namespace datagui;

  Window window;
  PointCloudShader point_cloud_shader;
  point_cloud_shader.init();

  Camera3d camera;
  camera.direction.x = std::cos(M_PI / 4);
  camera.direction.y = 0;
  camera.direction.z = -std::sin(M_PI / 4);
  camera.position.x = -2;
  camera.position.z = 2;
  camera.fov_degrees = 90;
  camera.clipping_min = 0.001;
  camera.clipping_max = 1000;

  struct Point {
    Vec3 position;
    Vec3 color;
  };
  std::vector<Point> points;
  for (float x = -1; x <= 1; x += 0.1) {
    for (float y = -1; y <= 1; y += 0.1) {
      Point point;
      point.position = Vec3(x, y, 0);
      point.color = Vec3((x + 1) / 2, (y + 1) / 2, 0);
      points.push_back(point);
    }
  }

  PointCloud point_cloud;
  point_cloud.load_points(
      points.data(),
      points.size(),
      offsetof(Point, position),
      offsetof(Point, color),
      sizeof(Point));

  while (window.running()) {
    window.render_begin();

    point_cloud_shader.queue_point_cloud(point_cloud, Vec3(), Rot3());
    point_cloud_shader.draw(window.size(), camera);
    point_cloud_shader.clear();

    window.render_end();
    window.poll_events();
  }
}
