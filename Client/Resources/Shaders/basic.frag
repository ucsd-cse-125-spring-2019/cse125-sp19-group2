#version 440 core

// Object color
uniform vec3 u_objCol = vec3(1, 1, 1);

out vec4 out_color;

void main(void)
{
  out_color = vec4(u_objCol, 1);
}
