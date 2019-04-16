#version 440 core

in vec2 uv;

// Object color
uniform vec3 u_objCol = vec3(1, 1, 1);

out vec4 out_color;

struct Material
{
  sampler2D diffuse;
  sampler2D specular;
};

uniform Material material;

void main(void)
{
  out_color = texture(material.diffuse, uv);
}
