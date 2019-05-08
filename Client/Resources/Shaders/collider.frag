#version 440 core

in vec3 pass_normal;

struct Material
{
  sampler2D diffuse;
  sampler2D specular;
};

uniform Material   u_material;

out vec4 out_color;

void main(void)
{
  out_color = vec4(pass_normal, 1.0f);
}
