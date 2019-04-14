#version 440 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

uniform mat4 u_projection = mat4(1);
uniform mat4 u_view = mat4(1);
uniform mat4 u_model = mat4(1);

void main(void)
{
  vec4 worldPosition = vec4(in_position, 1);
  gl_Position = u_projection * u_view * u_model * worldPosition;
}
