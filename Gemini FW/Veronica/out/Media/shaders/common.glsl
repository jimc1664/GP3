
#define ZPackVal  10

vec2 zPack( float a ) { 
	return vec2(a,0);
	//return vec2( floor(a*ZPackVal)/ZPackVal,  fract(a*ZPackVal) );
}
float zUnpack( vec2 a ) { 
	return a.x;
	//return (a.x + a.y/ZPackVal );
}
