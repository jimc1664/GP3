#ifndef GEM_DIS_OGL_TEXTUREIMP_PNG_H
#define GEM_DIS_OGL_TEXTUREIMP_PNG_H


#include <Gem/Math/Vec2.h>

typename FILE;
namespace Gem { namespace Dis {

class RawImageDat {
	RawImageDat(const u32 &width, const u32 &height, const u32 &format, const u32 &intFormat) : Size(width,height), Format(format), IntFormat(intFormat ) {}
	static RawImageDat*alloc( const u32 &width, const u32 &height, const u32 &stride, const u32 &format, const u32 &intFormat) {
		RawImageDat* ret = (RawImageDat*)malloc( sizeof(RawImageDat) + width*height*stride -1 );
		new (ret) RawImageDat(width, height, format, intFormat);
		return ret;
	}
public:
	static RawImageDat* fromPNG(FILE *fp );

	const vec2u Size;
	const u32 IntFormat, Format;

//private:
	u8 Dat[1];
};

}	}
#endif //GEM_DIS_OGL_TEXTUREIMP_PNG_H