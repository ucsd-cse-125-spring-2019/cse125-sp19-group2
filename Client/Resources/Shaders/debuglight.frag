#version 440 core

uniform vec3 u_color = vec3(0, 1, 0);

out vec4 out_color;

void main(void)
{
  out_color = vec4(u_color, 1.0f);
}
