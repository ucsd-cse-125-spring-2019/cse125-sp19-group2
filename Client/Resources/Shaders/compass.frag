#version 330 core

in vec3 pass_fragPos;
in vec3 pass_normal;
in vec2 pass_uv;

struct Material
{
  sampler2D diffuse;
};

uniform Material   u_material;

out vec4 out_color;

void main(){
	//out_color = texture(u_material.diffuse, pass_uv);
	out_color = vec4(1,1,1,1);
}
