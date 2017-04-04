#ifndef GEM_WING_INITCONSOLE_H
#define GEM_WING_INITCONSOLE_H

#include <iostream>
#include <io.h>
#include <fcntl.h>

namespace Gem {		namespace WinG {


	/**/
	//for png stuff  --- does not seem to work  --  keep for linkers piece of mind...
#pragma comment( lib, "legacy_stdio_definitions" )
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) {
	//FILE _iob[] = { *stdin, *stdout, *stdout };
	return _iob;
}

void initConsole() {

	AllocConsole();	

	
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	//freopen_s(&fp, "CONOUT$", "w", stderr);
	//_iob[1] = *fp;
	freopen_s(&fp, "CONIN$", "r", stdin);
	//_iob[0] = *fp;

	/*HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	s32 hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;
		*/
};

} }

#endif //GEM_WING_INITCONSOLE_H