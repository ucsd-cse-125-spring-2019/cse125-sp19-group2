#version 440 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 pass_fragPos;
out vec3 pass_normal;
out vec2 pass_uv;

void main(void)
{
  vec4 worldPosition = vec4(in_position, 1.0f);

  pass_fragPos = vec3(u_model * worldPosition);
  pass_normal  = mat3(transpose(inverse(u_model))) * in_normal;
  pass_uv = in_uv;

  gl_Position = u_projection * u_view * u_model * worldPosition;
}
