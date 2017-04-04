#version 150

#include "common.glsl"

out vec4 FragColor;

in vec3 Pos;
in vec2 UV;

uniform	sampler2D texture0;
uniform mat4 MVP, MV;  //actually this is the view...
uniform vec4 LightV;

vec4 lighting( vec3 nrm, vec3 ld, float att  ) {
	
	float specPow = 10;
	vec3 lightCol = vec3( 3,1,0.8 );
	
	vec3 vn = vec3(0.0, 0.0, 1.0);
	float dt = dot(ld, nrm ); 
	
	float spec = pow( clamp( dot(reflect(vn, nrm), ld), 0, 1), specPow );
	return vec4( lightCol*att, spec ) * (dt); 
	//float spec = dot(reflect(vn, nrm), ld);
	//return vec4( lightCol * (dt*att), spec ); 
}

void main() {

	vec4 smp = texture(texture0, UV);
	vec3 nrm = vec3(smp.x,smp.y, 0)*2 - 1; 
	nrm.z = -sqrt( 1- dot(nrm.xy,nrm.xy) );
	
	float depth = zUnpack( smp.zw )*0.5f;
	//if( depth > 0.9 ) discard;
	

	vec3 pos = normalize( Pos.xyz ) *depth;

	vec4 tp = vec4(  UV.x * 2 - 1,  (1 - UV.y) * 2 - 1, depth, 1.0f );
	//tp = mul( tp, MV );
	tp = MV *tp;
	tp.xyz /= tp.w;
	tp.w = 1;
	//tp = mul( tp, MVP );
	tp =  MVP *tp;
	pos = tp.xyz;

	vec3  lVec =  mat3(MVP)* LightV.xyz  - pos ;
	lVec = pos;
	//lVec = LightV.xyz  - pos;
	float magSqr = dot(lVec, lVec);
	float rad = LightV.w;

	rad = 12;
	FragColor = vec4(0,0,0,0);
	//FragColor.r = depth;
	FragColor.xyz = pos;
	//FragColor =  vec4( length(lVec)/rad,0,0,1); 
//	if( magSqr > rad *rad ) discard;

	float mag = sqrt(magSqr);
	lVec /= mag;
	//FragColor = max( lighting( nrm, lVec, 0.8 *(rad-mag) ), vec4(0,0,0,0) ) ;
//	FragColor =   max(  vec4(1 *0.8 *(rad-mag)  ,0,0,0), vec4(0,0,0,0) ); 

}
