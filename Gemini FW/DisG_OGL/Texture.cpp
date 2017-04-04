#include "stdafx.h"

#include "Gem/Dis/Texture.h"

#include "OGl_Task.h"

#include "Gem/String/DStr.h"

#include "TextureImp/Png.h"

#include <Gem/ConCur/WorkHost.h>

/*
namespace Gem {
	class Rsrc_Texture : public Resource_T<Rsrc_Texture> {
		friend class Entry;
		Rsrc_Texture(Entry &e, const u32 s) : Resource_T(e), Sz(s) {}
		static void fromFile(WorkHost&, Entry &e, CallBackHndl &&uCb) {
			//ws->init(e, uCb.move(), Rsrc_Raw::fromFile(e.Name, cb.copy()));			
		}
		const u32 Sz;
		char Data[1];
	public:
		
		static Hndl fromFile(const CStr &s, CallBackHndl cb = Nothing()) { return Entry::fromFile(s, cb.move()); }
	};
}
*/



#include "OGlMain.h"

Texture::Texture() {}

void checkError();



//void Texture::fromFile( DisMain &dm, const CStr &s) { }


void checkError();

void Texture::gen(DisMain &dm, const RawImageDat& id ){

	
	glGenTextures(1, &TexID);
	glBindTexture(GL_TEXTURE_2D, TexID);
        
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 	
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(
        GL_TEXTURE_2D, 0, id.IntFormat, id.Size.x, id.Size.y,
        0, id.Format, GL_UNSIGNED_BYTE, id.Dat
    );

	glGenerateMipmap(GL_TEXTURE_2D);

	//
	checkError();
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture(GL_TEXTURE_2D, 0);
}


void Texture::apply(RenderingCntx &dm ) {
	glBindTexture(GL_TEXTURE_2D, TexID);
}


void Texture::fromFile(WorkHost& cntx, const CStr &s) {
	
	RawImageDat *imgD; ///todo - not raw pointer

	FILE *fp = 0;
	fopen_s(&fp, s, "rb");
	if (!fp)
		Error("FileNotFound");

	auto fEx = getFileType(s);

	if (fEx.isEqual(CSTR("png"))) {
		imgD = RawImageDat::fromPNG(fp);
	} else {
		Error("Unknown Extension");
		fclose(fp);
		return;
	}
	fclose(fp);

	auto _this = this;
	TaskSchedule(DisMain,
		_this->gen(cntx, *imgD);
		delete imgD;
	);
}

Texture* Texture::fromFile(const CStr &_s) {
	
	auto ret = new Texture;
	DStr s2 = _s;

	TaskSchedule(WorkHost,
		ret->fromFile(cntx, s2);
	);


	return ret;
}
