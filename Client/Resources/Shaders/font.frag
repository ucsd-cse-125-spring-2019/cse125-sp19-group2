#version 330 core

in vec2 texcoord;

uniform sampler2D tex;
uniform vec4 textColor;

out vec4 fragColor;

void main(void) {
  fragColor = vec4(texture2D(tex, texcoord).r) * textColor;
  //fragColor = color;
}
