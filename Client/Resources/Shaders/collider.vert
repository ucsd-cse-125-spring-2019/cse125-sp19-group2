#version 440 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 pass_normal;

void main(void)
{
  vec4 worldPosition = vec4(in_position, 1.0f);
  pass_normal = in_normal;
  gl_Position = u_projection * u_view * u_model * worldPosition;
}
