#version 330 core

in vec3 fs_normal_cs;
in vec3 fs_normal_ws;
in vec4 fs_color;
flat in int fs_instance_id;
out vec4 color;

void main(){
  const vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
  // https://godotshaders.com/shader/transparency-dither/
  // + Offset by fs_instance_id x some magic number
	if (fs_color.a < 0.001 ||
      fs_color.a < fract(magic.z * (0.05f * fs_instance_id + fract(dot(gl_FragCoord.xy, magic.xy))))) {
		discard;
	}

  float Ka = 0.2;
  float Kc = 0.5;
  float Kw = 0.3;
  float camera_light = clamp(dot(fs_normal_cs, vec3(0, 0, 1)), 0, 1);
  float world_light = clamp(dot(fs_normal_ws, vec3(0, 0, 1)), 0, 1);
  color = fs_color * min(Ka + Kc * camera_light + Kw * world_light, 1);
}
