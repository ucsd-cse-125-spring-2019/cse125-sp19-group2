#version 440 core

in vec2 pass_uv;

struct Material
{
  sampler2D diffuse;
};

uniform Material u_material;

// Output
out vec4 out_color;

void main(void)
{ 
  out_color = vec4(texture(u_material.diffuse, pass_uv).rgb, 1.0f);
}
