#version 330 core

in vec2 fs_position_ms;
flat in vec4 fs_color;
flat in vec4 fs_border_color;
flat in vec2 fs_border_width;
flat in vec2 fs_radius;

out vec4 color;

void main() {
  vec2 size = vec2(0.5, 0.5);
  vec2 pos = abs(fs_position_ms);
  vec2 arc_origin = size - fs_radius;
  vec2 arc_pos = pos - arc_origin;

  if (fs_radius.x > 0 && fs_radius.y > 0
      && arc_pos.x >= 0 && arc_pos.y >= 0)
  {
    vec2 arc_pos_outer = arc_pos / fs_radius;
    vec2 arc_pos_inner = arc_pos / (fs_radius - fs_border_width);

    if (length(arc_pos_outer) > 1) {
      discard;
    } else if (length(arc_pos_inner) > 1) {
      color = fs_border_color;
    } else {
      color = fs_color;
    }
  } else {
    if (pos.x > size.x || pos.y > size.y) {
      discard;
    }
    if (pos.x > size.x - fs_border_width.x
        || pos.y > size.y - fs_border_width.y) {
      color = fs_border_color;
    } else {
      color = fs_color;
    }
  }
}
