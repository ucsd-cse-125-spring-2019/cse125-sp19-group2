#version 330 core

in vec2 pass_uv;

out vec4 out_color;

uniform sampler2D u_urine_sprite;

void main(void)
{
  out_color = texture(u_urine_sprite, pass_uv);
}