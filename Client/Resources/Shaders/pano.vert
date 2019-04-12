#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 position;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out float z;
out vec3 panoPosition;


void main(){
	panoPosition = position;
	gl_Position =  vec4(position,1);
	UV = (position.xy+vec2(1,1))/2.0;
}
