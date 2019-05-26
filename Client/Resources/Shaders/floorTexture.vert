#version 440 core

layout(location = 0) in vec3 in_position;
layout(location = 2) in vec2 in_uv;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

out vec2 pass_uv;

void main(void)
{
  vec4 worldPosition = vec4(in_position, 1.0f);
  worldPosition.x = worldPosition.x;

  pass_uv = in_uv;

  gl_Position = u_projection * u_view * u_model * worldPosition;
}
