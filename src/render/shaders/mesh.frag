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
