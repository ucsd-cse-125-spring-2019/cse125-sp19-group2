#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_particle_position;

out vec2 pass_uv;

uniform mat4 u_projection;
uniform mat4 u_view;

uniform vec2 u_size;

void main(void)
{
  vec3 camera_right = vec3(u_view[0][0], u_view[1][0], u_view[2][0]);
  vec3 camera_up = vec3(u_view[0][1], u_view[1][1], u_view[2][1]);

  vec3 position = in_particle_position
    + camera_right * in_position.x * u_size.x
	+ camera_up * in_position.y * u_size.y;

  pass_uv = vec2(in_position.x + 0.5f, in_position.y + 0.5f);

  gl_Position = u_projection * u_view * vec4(position, 1.0f);
}
