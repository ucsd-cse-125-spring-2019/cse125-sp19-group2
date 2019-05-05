#version 440 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;
layout (location = 3) in ivec4 in_boneid;
layout (location = 4) in ivec4 in_boneid2;
layout (location = 5) in vec4 in_weight;
layout (location = 6) in vec4 in_weight2;

out vec3 pass_fragPos;
out vec3 pass_normal;
out vec2 pass_uv;

const int MAX_BONES = 100;

uniform mat4 u_projection; 
uniform mat4 u_view; 
uniform mat4 u_model;
uniform mat4 u_bones[MAX_BONES];

void main()
{       
    mat4 boneTransform = u_bones[in_boneid[0]] * in_weight[0];
    boneTransform += u_bones[in_boneid[1]] * in_weight[1];
    boneTransform += u_bones[in_boneid[2]] * in_weight[2];
    boneTransform += u_bones[in_boneid[3]] * in_weight[3];
	boneTransform += u_bones[in_boneid2[0]] * in_weight2[0];
    boneTransform += u_bones[in_boneid2[1]] * in_weight2[1];
    boneTransform += u_bones[in_boneid2[2]] * in_weight2[2];
    boneTransform += u_bones[in_boneid2[3]] * in_weight2[3];

    vec4 pos = boneTransform * vec4(in_position, 1.0);
    pass_fragPos = (u_model * pos).xyz; 
    gl_Position = u_projection * u_view * u_model * pos;
    vec4 normal = boneTransform * vec4(in_normal, 0.0);
    pass_normal = (u_model * normal).xyz;
    pass_uv = in_uv;
}
