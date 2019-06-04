#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 position;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

uniform mat4 u_model;

void main() {
	gl_Position = u_model * vec4(position, 1.0f);

	// setting UV (don't change)
	UV = (position.xy+vec2(1,1))/2.0;
	UV.y = (UV.y - 1) * -1;
}
