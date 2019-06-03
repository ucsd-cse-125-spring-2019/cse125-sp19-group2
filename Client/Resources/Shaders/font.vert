#version 330 core

layout(location = 0) in vec4 coord;
uniform mat4 ProjectionMatrix = mat4(1);
uniform mat4 ModelView = mat4(1);
uniform mat4 ScaleMat = mat4(1);
out vec2 texcoord; 

void main(void) {
  gl_Position = ProjectionMatrix * ModelView * ScaleMat * vec4(coord.xy, 0, 1);
  texcoord = coord.zw;
}