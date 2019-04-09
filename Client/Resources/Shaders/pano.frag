#version 330 core

in vec2 UV;

out vec4 color;

uniform mat4 projection;
uniform mat4 view;
uniform sampler2D rgbTexture;

uniform float debugEdge;

in vec3 panoPosition;


const float PI = 3.1415926535897932384626433832795;
const float innerEdge = 0.85;
const float outerEdge = 0.90;

float atan2(in float y, in float x)
{
    bool s = (abs(x) > abs(y));
    return mix(PI/2.0 - atan(x,y), atan(y,x), s);
}


//x = (yaw + 180)/360 * width and y = (90 - pitch)/180 * height.
//yaw=theta
void main(){

	float x = panoPosition.x/2;
	float y = panoPosition.y/2;
	float yaw = (2*PI*x/1);
	float pitch = (y/1*PI);
	
	float cx = 1 * cos(pitch) * sin(yaw);
	float cy = 1 * sin(pitch);
	float cz = 1 * cos(pitch) * cos(yaw);
	
	vec4 worldSpaceCoord = vec4(cx,cy,cz,1);
	worldSpaceCoord = projection * mat4(mat3(view)) * worldSpaceCoord; // mat4(mat3(view))
	worldSpaceCoord = worldSpaceCoord / worldSpaceCoord.w;
	vec2 finalUV = worldSpaceCoord.xy;
	finalUV = (finalUV+vec2(1,1))/2.0;
	//finalUV.y = 1 - finalUV.y;
	bool flag = (worldSpaceCoord.x > 1 || worldSpaceCoord.x < -1 || worldSpaceCoord.y > 1 || worldSpaceCoord.y < -1 ||worldSpaceCoord.z > 1 || worldSpaceCoord.z < -1);
	bool flag2 = ((worldSpaceCoord.x > innerEdge && worldSpaceCoord.x < outerEdge && worldSpaceCoord.y > -innerEdge && worldSpaceCoord.y < innerEdge)
					|| (worldSpaceCoord.x < -innerEdge && worldSpaceCoord.x > -outerEdge && worldSpaceCoord.y > -innerEdge && worldSpaceCoord.y < innerEdge) 
					|| (worldSpaceCoord.y > innerEdge && worldSpaceCoord.y < outerEdge && worldSpaceCoord.x > -innerEdge && worldSpaceCoord.x < innerEdge) 
					|| (worldSpaceCoord.y < -innerEdge && worldSpaceCoord.y > -outerEdge && worldSpaceCoord.x > -innerEdge && worldSpaceCoord.x < innerEdge) );
	//flag = false;
	flag2 = flag2 && (debugEdge >= 1);
	if(flag){
		color = vec4(0,0,0,0);
	}else if(flag2){
		color = vec4(0,1,0,1);
	}else{
		color = texture( rgbTexture, finalUV );
	}
	//color = vec4(x,y ,0,1);
	
}
