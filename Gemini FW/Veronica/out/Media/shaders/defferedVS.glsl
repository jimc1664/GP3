#version 150

in vec3 vertexPosition;

out vec2 UV;
out vec3 Pos;
out vec2 DepthPass;

uniform mat4 MVP;

void main() {
	UV=(vertexPosition.xy + 1)*vec2(0.5,0.5);


	vec4 wPos = vec4( Pos = vertexPosition, 1.0);

	gl_Position = wPos;
	DepthPass = wPos.zw;
	
}
