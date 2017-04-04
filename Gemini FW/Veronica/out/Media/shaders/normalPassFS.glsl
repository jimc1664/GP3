#version 150

#include "commonDefferedFS.glsl"



void main() {

	vec3 nrm = normalize(  Norm )*0.5 +vec3(0.5,0.5,0.5);
//	vec3 pos = VPos*0.5 +vec3(0.5,0.5,0.5);
	float depth = VPos.z / VPos.w;
	FragColor = vec4(0,0,0,1);
	//depth = 0;
	depth = gl_FragCoord.z;
	FragColor = vec4(nrm.x, nrm.y, zPack(depth) );
	//FragColor = vec4(nrm.x, nrm.y, pos.z, 0 );
	
	//FragColor = vec4(nrm.x, nrm.y, nrm.z, 0 );
	//FragColor = vec4(pos.x, pos.y, pos.z, 0 );
}
