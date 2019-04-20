#version 440 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

out vec2 uv;

uniform mat4 u_projection = mat4(1);
uniform mat4 u_view = mat4(1);
uniform mat4 u_model = mat4(1);

void main(void)
{
  uv = in_uv;
  vec4 worldPosition = vec4(in_position, 1);
  gl_Position = u_projection * u_view * u_model * worldPosition;
}
