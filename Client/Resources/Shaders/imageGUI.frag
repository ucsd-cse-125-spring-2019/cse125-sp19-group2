#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D rgbTexture;
uniform float alpha = 1;

void main(){
	color = texture( rgbTexture, UV ) * vec4(1, 1, 1, alpha);
}
