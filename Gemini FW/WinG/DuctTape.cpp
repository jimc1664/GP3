#include "stdafx.h"

#ifdef DEBUG
#include <Gem/OS/DuctTape.h>

#include "dbghelp.h"
#pragma comment(lib, "dbghelp.lib" )

#include "Gem/String/WStr.h"
#include "Gem/String/DebugOutput.h"
#include "Gem/ConCur/CritSec.h"
#include "Gem/ConCur/Thread.h"  //id

#pragma warning(disable :  4074)
#pragma init_seg( compiler  )


#define OUTPUT(a) DebugOutput()<<a

namespace Gem { namespace DuctTape {

	void outputExtendedError( const StringUkn &str ) {
			
		DWORD ec = GetLastError();

		if( ec ) {
			LPVOID lpMsgBuf;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, ec, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL);

			OUTPUT( str<<"  coz  "<<(char*)lpMsgBuf );

			LocalFree(lpMsgBuf);
			SetLastError( 0);


		} else OUTPUT( str<<"  for reasons unknown \n");
	}


	class SymbolMan { //loads and manages symbol data
	public:
		SymbolMan(bool unloadSymbolsAfter) : ProcessHndl(GetCurrentProcess()) {	
			if(!SymbolsLoaded ) loadSymbols();

			SYMBOL_INFO *symbolData = (SYMBOL_INFO*)&SymbolData;
			symbolData->SizeOfStruct = sizeof(SYMBOL_INFO);
			symbolData->MaxNameLen = SymbolMaxNameLen;
			FileLineData.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
		}
		void printCallStack( const SIZE_T callStack[], const u32 &callStackCount ) {
			SYMBOL_INFO *symbolData = (SYMBOL_INFO*)&SymbolData;
			for(u32 i = 0; i < callStackCount; i++) {
				if( SymFromAddr(ProcessHndl, callStack[i], NULL, symbolData)) 
					OUTPUT( "func := "<<symbolData->Name);
				else {
				//	OUTPUT( "Failed to find function name!");			
					outputExtendedError( CSTR("Failed to find function name!  "));	
				}
				DWORD dwDisplacement;
				if( SymGetLineFromAddr64(ProcessHndl, callStack[i], &dwDisplacement, &FileLineData ) ) {
					OUTPUT("  Line := "<<FileLineData.LineNumber<<"  File := "<<FileLineData.FileName<<"\n");
				} else {
					OUTPUT("  File and line number not available.  iPtr := "<<(void*)callStack[i]<<"\n");
				}
			}
		}
		~SymbolMan() {
			if( UnloadSymbolsAfter ) {
				HANDLE hProcess = GetCurrentProcess();
				EnumerateLoadedModulesW64( hProcess, unloadModuleCB, hProcess );
				SymbolsLoaded = false;
			}
		}
	private:
		static BOOL __stdcall unloadModuleCB( PCWSTR modName, DWORD64 modAddr, ULONG modSize, PVOID context ) {
			HANDLE hProcess = (HANDLE)context;
			IMAGEHLP_MODULEW64  moduleimageinfo;  moduleimageinfo.SizeOfStruct = sizeof(moduleimageinfo);
			if( SymGetModuleInfoW64(hProcess, modAddr,&moduleimageinfo) != TRUE) {
				SymUnloadModule64(hProcess, modAddr);
			}
			return TRUE;
		}
	/*	static void failedToLoadSymbols(  WStr<MAX_PATH> &mn ) {
			
			DWORD ec = GetLastError();

			if( ec ) {
				LPVOID lpMsgBuf;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, ec, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL);

				OUTPUT( "Failed to load symbols for "<<mn<<"  coz  "<<(char*)lpMsgBuf<<" \n");

				LocalFree(lpMsgBuf);
				SetLastError( 0);


			} else OUTPUT( "Failed to load symbols for "<<mn<<"  for reasons unknown \n");
		}*/

		static BOOL __stdcall loadedModuleCB( PCWSTR modName, DWORD64 modAddr, ULONG modSize, PVOID context ) {
			HANDLE hProcess = (HANDLE)context;
			 
			WStr<MAX_PATH> modName2( modName );
	
			IMAGEHLP_MODULEW64  moduleimageinfo;  moduleimageinfo.SizeOfStruct = sizeof(moduleimageinfo);
			if( SymGetModuleInfoW64(hProcess, modAddr,&moduleimageinfo) != TRUE) {
				if( SymLoadModule64(hProcess, NULL, modName2, NULL, modAddr, modSize) == 0 ) {
					outputExtendedError( CSTR("Failed to load symbols for "));					
				} if( SymGetModuleInfoW64(hProcess, modAddr,&moduleimageinfo) != TRUE) 
					outputExtendedError( CSTR("Failed to load symbols for "));			
			}
			return TRUE;
		}
		static void loadSymbols() { 
			WStrW<MAX_PATH*4> pathList;
			pathList.appendFnc( GetModuleFileNameW, GetModuleHandle(NULL) );
			pathList.fileToDir();

			pathList += CSTRW( L";.\\" ); //working directory
			(pathList += L';').appendFnc( GetWindowsDirectoryW );
			(pathList += L';').appendFnc( GetSystemDirectoryW );
			(pathList += L';').appendFnc( GetEnvironmentVariableW, (LPCWSTR)"_NT_ALT_SYMBOL_PATH" );
			(pathList += L';').appendFnc( GetEnvironmentVariableW, (LPCWSTR)"_NT_SYMBOL_PATH" );

			HANDLE hProcess = GetCurrentProcess();
			SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
			AssertOnFail( SymInitializeW(hProcess, pathList, FALSE), "DuctTape:: Failed to load symbols." );
			EnumerateLoadedModulesW64( hProcess, loadedModuleCB, hProcess );
			SymbolsLoaded = true;
		}
		const HANDLE		ProcessHndl;
		static const u32	SymbolMaxNameLen = 256;
		u8					SymbolData[sizeof(SYMBOL_INFO)+SymbolMaxNameLen-1];
		IMAGEHLP_LINE64		FileLineData;
		bool				UnloadSymbolsAfter;
		static bool			SymbolsLoaded;
	};
	bool SymbolMan::SymbolsLoaded = false;

	CritSec Lock;
	//be careful with any heap functions in here

#pragma pack(push, 1 )
	class HeapBlock{
	public:
		class InitData : Uncopyable {
		public:


			static void printIP( SIZE_T ip  ) {
				static const u32	SymbolMaxNameLen = 256;
				u8					SymbolData[sizeof(SYMBOL_INFO)+SymbolMaxNameLen-1];
				auto ProcessHndl = GetCurrentProcess();
				IMAGEHLP_LINE64		FileLineData;

				SYMBOL_INFO *symbolData = (SYMBOL_INFO*)&SymbolData;
			//	for(u32 i = 0; i < callStackCount; i++) {
					if( SymFromAddr( ProcessHndl, ip, NULL, symbolData)) 
						OUTPUT( "func := "<<symbolData->Name);
					else {
					//	OUTPUT( "Failed to find function name!");
						OUTPUT("Failed to find function name!  iPtr := "<<(void*)ip<<"\n");			
						outputExtendedError( CSTR(" "));	
					}
					DWORD dwDisplacement;
					if( SymGetLineFromAddr64(ProcessHndl, ip, &dwDisplacement, &FileLineData ) ) {
						OUTPUT("  Line := "<<FileLineData.LineNumber<<"  File := "<<FileLineData.FileName<<"\n");
					} else {
						OUTPUT("  File and line number not available.  iPtr := "<<(void*)ip<<"\n");
					}
				
			}

			//walking the callstack walking the green callstack, also we will work out some sizes
			InitData( const sizet &reqSize ) : Count(0) {	
				
				/*static bool tFlag = true;
				if( !tFlag ) {  //guard against infinite recursion, for instance for when dumbass's like myself output to the console in here
					calcSize(reqSize);
					return; 
				} tFlag = false; */

				//SymbolMan sm(true);
#if defined(_M_IX86)  //Creates variable 'a' storing the frame pointer 
				SIZE_T* curFP; 
				{__asm mov curFP, Ebp }

				for( SIZE_T* nextFP = (SIZE_T*)*curFP; nextFP != NULL; curFP = nextFP, nextFP = (SIZE_T*)*curFP ) {
					if( nextFP < curFP )							break; // TODO -handle error :: Frame pointer addresses should always increase as we move up the stack.
					if( (SIZE_T)*curFP & (sizeof(SIZE_T*) - 1) )	break; // TODO -handle error :: Frame pointer addresses should always be aligned to the size of a pointer. This probably means that 
																			// --   we've encountered a frame that was created by a module built with frame pointer omission (FPO) optimization turned on
					if( IsBadReadPtr( nextFP, sizeof(SIZE_T*) ) )	break; // TODO -handle error :: Bogus frame pointer. Again, this probably means that we've encountered a frame built with FPO optimization.
					FrameAddr[Count] = *(curFP + 1);			
					if( Count++ == MaxCount ) break;
				}
#elif defined (_M_X64)
				/*SIZE_T* curFP; 
				{
					CONTEXT context;
					RtlCaptureContext(&context);
					curFP = (DWORD_PTR*)context.Rip;
				} 

				for( SIZE_T* nextFP = (SIZE_T*)*curFP; nextFP != NULL; curFP = nextFP, nextFP = (SIZE_T*)*curFP ) {
					if( nextFP < curFP )							break; // TODO -handle error :: Frame pointer addresses should always increase as we move up the stack.
					if( (SIZE_T)*curFP & (sizeof(SIZE_T*) - 1) )	break; // TODO -handle error :: Frame pointer addresses should always be aligned to the size of a pointer. This probably means that 
																			// --   we've encountered a frame that was created by a module built with frame pointer omission (FPO) optimization turned on
					if( IsBadReadPtr( nextFP, sizeof(SIZE_T*) ) )	break; // TODO -handle error :: Bogus frame pointer. Again, this probably means that we've encountered a frame built with FPO optimization.
					FrameAddr[Count] = *(curFP + 1);			
					if( Count++ == MaxCount ) break;
				}*/
typedef struct _tagSTACKFRAME64 {
    ADDRESS64   AddrPC;               // program counter
    ADDRESS64   AddrReturn;           // return address
    ADDRESS64   AddrFrame;            // frame pointer
    ADDRESS64   AddrStack;            // stack pointer
    ADDRESS64   AddrBStore;           // backing store pointer
    PVOID       FuncTableEntry;       // pointer to pdata/fpo or NULL
    DWORD64     Params[4];            // possible arguments to the function
    BOOL        Far;                  // WOW far call
    BOOL        Virtual;              // is this a virtual frame?
    DWORD64     Reserved[3];
    KDHELP64    KdHelp;
} STACKFRAME64, *LPSTACKFRAME64;
				CONTEXT c;
				RtlCaptureContext(&c);

				STACKFRAME64 s;
  s.AddrPC.Offset = c.Rip;
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Offset = c.Rsp;
  s.AddrFrame.Mode = AddrModeFlat;
  s.AddrStack.Offset = c.Rsp;
  s.AddrStack.Mode = AddrModeFlat;
	
  s.AddrStack.Offset = c.Rbp;
	

  printIP(*((SIZE_T*)c.Rip + 1));
  printIP( c.Rip );

  SIZE_T* curFP;
		

		curFP = (DWORD_PTR*)c.Rip;

		for( SIZE_T* nextFP = (SIZE_T*)*curFP; nextFP != NULL; curFP = nextFP, nextFP = (SIZE_T*)*curFP ) {
			if( nextFP < curFP )							break; // TODO -handle error :: Frame pointer addresses should always increase as we move up the stack.
			if( (SIZE_T)*curFP & (sizeof(SIZE_T*) - 1) )	break; // TODO -handle error :: Frame pointer addresses should always be aligned to the size of a pointer. This probably means that 
																	// --   we've encountered a frame that was created by a module built with frame pointer omission (FPO) optimization turned on
			if( IsBadReadPtr( nextFP, sizeof(SIZE_T*) ) )	break; // TODO -handle error :: Bogus frame pointer. Again, this probably means that we've encountered a frame built with FPO optimization.

			printIP( *(curFP + 1) );
			if( Count++ == MaxCount ) break;
		}

		Count = 0;
		curFP = (DWORD_PTR*)c.Rbp;
		for( SIZE_T* nextFP = (SIZE_T*)*curFP; nextFP != NULL; curFP = nextFP, nextFP = (SIZE_T*)*curFP ) {
			if( nextFP < curFP )							break; // TODO -handle error :: Frame pointer addresses should always increase as we move up the stack.
			if( (SIZE_T)*curFP & (sizeof(SIZE_T*) - 1) )	break; // TODO -handle error :: Frame pointer addresses should always be aligned to the size of a pointer. This probably means that 
																	// --   we've encountered a frame that was created by a module built with frame pointer omission (FPO) optimization turned on
			if( IsBadReadPtr( nextFP, sizeof(SIZE_T*) ) )	break; // TODO -handle error :: Bogus frame pointer. Again, this probably means that we've encountered a frame built with FPO optimization.

			printIP( *(curFP + 1) );
			if( Count++ == MaxCount ) break;
		}
		Count = 0;
		curFP = (DWORD_PTR*)c.Rsp;
		for( SIZE_T* nextFP = (SIZE_T*)*curFP; nextFP != NULL; curFP = nextFP, nextFP = (SIZE_T*)*curFP ) {
			if( nextFP < curFP )							break; // TODO -handle error :: Frame pointer addresses should always increase as we move up the stack.
			if( (SIZE_T)*curFP & (sizeof(SIZE_T*) - 1) )	break; // TODO -handle error :: Frame pointer addresses should always be aligned to the size of a pointer. This probably means that 
																	// --   we've encountered a frame that was created by a module built with frame pointer omission (FPO) optimization turned on
			if( IsBadReadPtr( nextFP, sizeof(SIZE_T*) ) )	break; // TODO -handle error :: Bogus frame pointer. Again, this probably means that we've encountered a frame built with FPO optimization.

			printIP( *(curFP + 1) );
			if( Count++ == MaxCount ) break;
		}
				
/*        LPCBYTE instructionPointer;
        instructionPointer=(LPCBYTE)c.Rip;*/			

#else
#error "Unhandled"
#endif

				calcSize(reqSize);


			}
			const sizet& RequiredMemory() const { return TotalSize; }
		private: friend class HeapBlock;
			SIZE_T*		callStackAddr( HeapBlock* hb ) const { return (SIZE_T*)((u8*)hb+TotalSize-Count*sizeof(SIZE_T)); }
			void		copyCallStack( HeapBlock* hb ) const { memcpy( callStackAddr(hb), FrameAddr, Count*sizeof(SIZE_T) ); }
			void		calcSize( const sizet &reqSize ) { 
//				Assume((size_t)sizeof(HeapBlock) == HeapBlock::DesSizeOf);
				TotalSize = sizeof(HeapBlock) + reqSize + Count*sizeof(SIZE_T); 
			}
			static const u32 MaxCount = 32;
			SIZE_T FrameAddr[MaxCount];
			sizet TotalSize;
			u32 Count;
		};  

		static HeapBlock*	realPtr( const ptr &p )			{ HeapBlock* ret = ((HeapBlock*)p)-1; ret->assertValidity(); return ret; }
		static ptr			init( const ptr &p, const InitData &id ) {
			HeapBlock *t = (HeapBlock*)p;
			new (t) HeapBlock(id);
			return t->retPtr();
		}
		static ptr			update( const ptr &p, const HeapBlock *const &oldP, const InitData &id ) {
			HeapBlock *t = (HeapBlock*)p;
			if( t != oldP ) {
				t->fix(id);
			} else 	t->Size = id.RequiredMemory(); //leave the callstack as is, err.. easier, other wise we may have to realllocate
			return t->retPtr();
		}
		static ptr			deinit( const ptr &p ) {
			HeapBlock *t = realPtr(p);
			t->~HeapBlock();
			return t;
		}
		static void			printAll(bool unloadSymbolsAfter) { 

			if(Last) {
				SymbolMan sm(unloadSymbolsAfter);
				for( HeapBlock* h = Last; h; h = h->Prev ) h->print(sm); 
				Beep( 8000, 400 );
				Beep( 4000, 500 );
			}
		}

	private:
		HeapBlock( const InitData &id ) : Size(id.RequiredMemory()), Next(0), Prev(Last), MagicNo(MagicNumber) { 
			initCallStack(id);
			//PRINT("Initing "<<this<<"  prev "<<Prev<<"\n");
			if(Prev) {
				Prev->assertValidity();
				Prev->Next = this;  
			}
			Last = this; 
		}
		void initCallStack( const InitData &id ) {
			CallStackCount = id.Count;
			CallStack = id.callStackAddr(this);
			id.copyCallStack(this);
		}
		~HeapBlock() {
			//IF_RECORDUNLINKING(UnlinkFile <<this<<"  -- "<<(void*)(((u8*)this)+Size+sizeof(HeapBlock))<<"  F "<<File<< " L "<<Line<<"\n");

			//if(Prev) Prev->CheckP(this);
			//if(Next) Next->CheckN(this);

			if(Next) {
				Next->Prev = Prev; 
			} else {
				Last = Prev;
			}
			if(Prev) {
				Prev->Next = Next;
			}
			//Next = Prev = 0;
			//MagicNo = 0;
		}
		void fix( const InitData &id ) {
			if( Next ) 
				Next->Prev = this;
			else 
				Last = this;
			if( Prev ) Prev->Next = this;
			initCallStack(id);
			assertValidity();
		}
		void assertValidity() { //not exactly foolproof...but debug allocaters do their stuff too..
			Assert( MagicNo == MagicNumber,				"HeapBlock:: 'this' is corrupted" );
			if( Prev ) {
				Assert( Prev->MagicNo == MagicNumber,	"HeapBlock:: 'Prev' is corrupted" );
				Assert( Prev->Next == this,				"HeapBlock:: 'Prev' is corrupted" );
			}
			if( Next ) {
				Assert( Next->MagicNo == MagicNumber,	"HeapBlock:: 'Next' is corrupted" );
				Assert( Next->Prev == this,				"HeapBlock:: 'Next' is corrupted" );
			} else {
				Assert( Last == this,					"HeapBlock:: has no 'Next' yet 'Last != this'" );
			}
		} 
		void	print( SymbolMan &sm ) { 		
			OUTPUT(" HeapBlock @ "<<this<<"  Size "<<Size<<"  CallStackCount "<<CallStackCount<<"\n"); 
			sm.printCallStack( CallStack, CallStackCount );
			OUTPUT("\n");
		}
		ptr		retPtr()			{ return this+1; }

		
		u64 MagicNo;
		size_t Size;
		HeapBlock *Next, *Prev;
		SIZE_T* CallStack;
		u32 CallStackCount;
	public:
		static HeapBlock* Last;
		static u64 MagicNumber;

#ifdef _WIN64
		static const size_t DesSizeOf = 64; //neat alignment
#else
		static const size_t DesSizeOf = 32; //neat alignment
#endif
	private:	
		u8 pad[DesSizeOf-sizeof(u64)-sizeof(size_t)-sizeof(SIZE_T*)-sizeof(u32)-sizeof(HeapBlock*)*2];
		void zNeverToBeCalled() { static_assert(  (size_t)sizeof( HeapBlock) == HeapBlock::DesSizeOf, "Size mismath" ); }
	}; 
#pragma pack(pop)

	HeapBlock* HeapBlock::Last = 0;
	u64	HeapBlock::MagicNumber = 13371337133713371337U ^ (size_t)&Last;

	#define TABLE _AllocWrapper_Tbl
	FARPROC TABLE[] = { 0,0,0,0 };
	template<int i> struct AllocWrapper {
		typedef void* (__cdecl *Proc_t)(size_t);
		static void* __cdecl func( size_t s ) {
			CritSec::Handle h(Lock);
			HeapBlock::InitData id(s);
			auto ret =HeapBlock::init(((Proc_t)TABLE[i])(id.RequiredMemory()), id);
			//printf("**allocWrapper %x  - Thread %d \n", (size_t)ret, Thread::id() );
			return ret;
		}
		static void set( FARPROC proc )	{ 
			Assert( i < ArySize(TABLE), "Out of bounds, make the table bigger?" );
			Assume(TABLE[i] == null || TABLE[i] == proc);
			TABLE[i] = proc; 
			//static int j = 0; Assert(j++ == 0, "this function should only be called once"); 
			return;
			Proc_t test = malloc; test = func;	//compiler check, make sure Proc_t is correct
		}
	#undef TABLE
	};

	#define TABLE _AllocDbgWrapper_Tbl
	FARPROC TABLE[] = { 0,0,0 };
	template<int i> struct AllocDbgWrapper {
		typedef void* (__cdecl *Proc_t)(size_t, int, const char*, int );
		static void* __cdecl func( size_t s, int b, const char *f, int l ) {
			CritSec::Handle h(Lock);
			HeapBlock::InitData id(s);
			auto ret = HeapBlock::init(((Proc_t)TABLE[i])(id.RequiredMemory(), b, f, l), id);
			//printf("**AllocDbgWrapper %x  - Thread %d \n", (size_t)ret, Thread::id());
			return ret;
		}
		static void set( FARPROC proc )	{ 
			Assert( i < ArySize(TABLE), "Out of bounds, make the table bigger?" );
			Assume(TABLE[i] == null || TABLE[i] == proc);
			TABLE[i] = proc;
//			static int j = 0; Assert(j++ == 0, "this function should only be called once"); 
			return;
			IF_DEBUG( Proc_t test = _malloc_dbg; test = func; ); //compiler check, make sure Proc_t is correct	
		}
	#undef TABLE
	}; 

	#define TABLE _ReallocWrapper_Tbl
	FARPROC TABLE[] = { 0 };
	template<int i> struct ReallocWrapper {
		typedef void* (__cdecl *Proc_t)(void*, size_t);
		static void* __cdecl func( void * p, size_t s ) {
			CritSec::Handle h(Lock);
			HeapBlock::InitData id(s);
			if( p ) {
				HeapBlock* old = HeapBlock::realPtr( p );
				return HeapBlock::update( ((Proc_t)TABLE[i])( old, id.RequiredMemory() ), old, id );
			} else {
				return HeapBlock::init( ((Proc_t)TABLE[i])( 0, id.RequiredMemory() ), id );
			}
		}
		static void set( FARPROC proc )	{ 
			Assert( i < ArySize(TABLE), "Out of bounds, make the table bigger?" );
			Assume(TABLE[i] == null || TABLE[i] == proc);
			TABLE[i] = proc;
//			static int j = 0; Assert(j++ == 0, "this function should only be called once"); 
			return;
			Proc_t test = realloc; test = func; //compiler check, make sure Proc_t is correct
		}
	#undef TABLE
	}; 

#define TABLE _FreeWrapper_Tbl
	FARPROC TABLE[] = { 0,0,0,0 };
	template<int i> struct FreeWrapper {
		typedef void(__cdecl *Proc_t)(void*);
		static void __cdecl func(void* p) {
			if (!p) return; //Mirror usual functionality
			CritSec::Handle h(Lock);
			//printf("**FreeWrapper %x  - Thread %d \n", (size_t)p, Thread::id());
			((Proc_t)TABLE[i])(HeapBlock::deinit(p));
		}
		static void set(FARPROC proc) {
			Assert(i < ArySize(TABLE), "Out of bounds, make the table bigger?");
			TABLE[i] = proc;
			Assume(TABLE[i] == null || TABLE[i] == proc);
			//			static int j = 0; Assert(j++ == 0, "this function should only be called once");		
			return;
			Proc_t test = free; test = func;//compiler check, make sure Proc_t is correct
		}
#undef TABLE
	};
#define TABLE _FreeDbgWrapper_Tbl
	FARPROC TABLE[] = { 0, };
	template<int i> struct FreeDbgWrapper {
		typedef void(__cdecl *Proc_t)(void*, int);
		static void __cdecl func(void* p, int blockType) {
			if (!p) return; //Mirror usual functionality
			CritSec::Handle h(Lock);
			//printf("**FreeDbgWrapper %x  - Thread %d \n", (size_t)p, Thread::id());
			((Proc_t)TABLE[i])(HeapBlock::deinit(p), blockType);
		}
		static void set(FARPROC proc) {
			Assert(i < ArySize(TABLE), "Out of bounds, make the table bigger?");
			TABLE[i] = proc;
			Assume(TABLE[i] == null || TABLE[i] == proc);
			//			static int j = 0; Assert(j++ == 0, "this function should only be called once");		
			return;
			Proc_t test = _free_dbg; test = func;//compiler check, make sure Proc_t is correct
		}
#undef TABLE
	};

	void replaceProcImportEntry( const HMODULE &clientMod, const CStr &serviceMod, const FARPROC &oldProc, const ptr &newProc) {		
		Assume( oldProc != newProc );

		//each module (the exe and each of its dll's) that references functions or global's does so indirectly and must look up the address on a table,
		// -- here we replace 'clientMod''s entry that refers to 'oldProc' of 'serviceMod' with 'newProc'

		ULONG size;
	//    EnterCriticalSection(&imagelock); //todo multithread

		IMAGE_IMPORT_DESCRIPTOR *iid = (IMAGE_IMPORT_DESCRIPTOR*)ImageDirectoryEntryToDataEx( clientMod, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size, NULL );
	   // LeaveCriticalSection(&imagelock);
		if(iid == NULL) return; //Failed  - could not not find a table refering to serviceMod

		for(;;iid++) {
			if( !iid->OriginalFirstThunk  )										return; // Failed! - to find relavent reference table 	
			if( isEqual(serviceMod, (char*)offsetPtr(clientMod, iid->Name) ) )	break; // these are droids you are looking for
		}
		
		for( IMAGE_THUNK_DATA *itd = (IMAGE_THUNK_DATA*)offsetPtr(clientMod, iid->FirstThunk); itd->u1.Function != 0x0; itd++ ) {
			if (itd->u1.Function == (size_t)oldProc) {
				DWORD oProtect;
				VirtualProtect( &itd->u1.Function, sizeof(itd->u1.Function), PAGE_READWRITE, &oProtect ); //ensure we are allowed to overwrite table entry
				itd->u1.Function = (size_t)newProc;
				VirtualProtect( &itd->u1.Function, sizeof(itd->u1.Function), oProtect, &oProtect );
				return; //Success
			}
		}
		//Fail - Could not find 'oldProc', maybe 'clientMod' doesn't actually use it, or we have already overwriten it
	}
	template<class WrapClass>
	void WrapProc( const HMODULE &cMod, const HMODULE &sMod, const CStr &sModName, const CStr &procName, const bool &critical =true ) {
		FARPROC proc = GetProcAddress( sMod, procName );

		if( !critical && proc == null ) return;
		Assert( proc != NULL, "Wrapping null proc??" );
		WrapClass::set( proc );
		replaceProcImportEntry( cMod, sModName, proc, WrapClass::func );
	}

	bool ProcSet = false;
	void foo() {}

	typedef LPVOID (WINAPI *OrigHeapAlloc_T)(HANDLE,DWORD, SIZE_T);
	OrigHeapAlloc_T OrigHeapAlloc;
	LPVOID
		WINAPI
		_my_HeapAlloc(
			_In_ HANDLE hHeap,
			_In_ DWORD dwFlags,
			_In_ SIZE_T dwBytes
			)
	{
		return OrigHeapAlloc(hHeap, dwFlags, dwBytes);
	}


	void activateForModule( HMODULE mod ) {
		auto mn = HeapBlock::MagicNumber;
		Assume( HeapBlock::MagicNumber == (13371337133713371337U ^ (size_t)&HeapBlock::Last) );
		if(!mod) {
			GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				  reinterpret_cast<LPCWSTR>(&foo),
				  &mod);
		}

#ifdef _M_IX86
		char* operatorSymbols[] = {
			"??2@YAPAXI@Z",			// new(uint)
			"??_U@YAPAXI@Z",		// new [] (uint)0
			"??3@YAXPAX@Z",			// delete(void*);
			"??_V@YAXPAX@Z",		// delete[](void*);
			"??2@YAPAXIHPBDH@Z",	// new(uint, const char*, int)
			"??_U@YAPAXIHPBDH@Z",	// new [] (uint, const char*, int)
		};
#elif _M_X64
		char* operatorSymbols[] = {
			"??2@YAPEAX_K@Z",	// new(uint)
			"??_U@YAPEAX_K@Z",	// new [] (uint)0
			"??3@YAXPEAX@Z",	// delete(void*);
			"??_V@YAXPEAX@Z",	// delete[](void*);
			"??2@YAPEAX_KPEBDH@Z",	// new(uint, const char*, int)
			"??_U@YAPEAX_KPEBDH@Z",	// new [] (uint, const char*, int)
		};
#endif
		/*
MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
MSVC++ 7.0  _MSC_VER == 1300
MSVC++ 6.0  _MSC_VER == 1200
MSVC++ 5.0  _MSC_VER == 1100
		 */
#if _MSC_VER == 1900
		auto msvcdll = CSTR("ucrtbased.dll");
#elif _MSC_VER == 1800
		auto msvcdll = CSTR("msvcr120d.dll");
#else 
#error  no symbols for this dll
#endif

		HMODULE msvcrXd = GetModuleHandleA(msvcdll.str());;
/*		char* m_patchTable[] = {
			/ * // Win32 heap APIs.
			"kernel32.dll",                                                                        // FALSE,  0x0, m_kernelbasePatch,                                                                        // we patch this record on Win7 and higher
			"kernel32.dll",                                                                        // FALSE,  0x0, m_kernel32Patch,

			// MFC new operators (exported by ordinal).
			"mfc42.dll",                                                                        //    TRUE,   0x0, mfc42Patch,
			"mfc42d.dll",                                                                        //   TRUE,   0x0, mfc42dPatch,
			"mfc42u.dll",                                                                        //   TRUE,   0x0, mfc42uPatch,
			"mfc42ud.dll",                                                                        //  TRUE,   0x0, mfc42udPatch,
			"mfc70.dll",                                                                        //    TRUE,   0x0, mfc70Patch,
			"mfc70d.dll",                                                                        //   TRUE,   0x0, mfc70dPatch,
			"mfc70u.dll",                                                                        //   TRUE,   0x0, mfc70uPatch,
			"mfc70ud.dll",                                                                        //  TRUE,   0x0, mfc70udPatch,
			"mfc71.dll",                                                                        //    TRUE,   0x0, mfc71Patch,
			"mfc71d.dll",                                                                        //   TRUE,   0x0, mfc71dPatch,
			"mfc71u.dll",                                                                        //   TRUE,   0x0, mfc71uPatch,
			"mfc71ud.dll",                                                                        //  TRUE,   0x0, mfc71udPatch,
			"mfc80.dll",                                                                        //    TRUE,   0x0, mfc80Patch,
			"mfc80d.dll",                                                                        //   TRUE,   0x0, mfc80dPatch,
			"mfc80u.dll",                                                                        //   TRUE,   0x0, mfc80uPatch,
			"mfc80ud.dll",                                                                        //  TRUE,   0x0, mfc80udPatch,
			"mfc90.dll",                                                                        //    TRUE,   0x0, mfc90Patch,
			"mfc90d.dll",                                                                        //   TRUE,   0x0, mfc90dPatch,
			"mfc90u.dll",                                                                        //   TRUE,   0x0, mfc90uPatch,
			"mfc90ud.dll",                                                                        //  TRUE,   0x0, mfc90udPatch,
			"mfc100.dll",                                                                        //   TRUE,   0x0, mfc100Patch,
			"mfc100d.dll",                                                                        //  TRUE,   0x0, mfc100dPatch,
			"mfc100u.dll",                                                                        //  TRUE,   0x0, mfc100uPatch,
			"mfc100ud.dll",                                                                        // TRUE,   0x0, mfc100udPatch,
			"mfc110.dll",                                                                        //   TRUE,   0x0, mfc110Patch,
			"mfc110d.dll",                                                                        //  TRUE,   0x0, mfc110dPatch,
			"mfc110u.dll",                                                                        //  TRUE,   0x0, mfc110uPatch,
			"mfc110ud.dll",                                                                        // TRUE,   0x0, mfc110udPatch,
			"mfc120.dll",                                                                        //   TRUE,   0x0, mfc120Patch,
			"mfc120d.dll",                                                                        //  TRUE,   0x0, mfc120dPatch,
			"mfc120u.dll",                                                                        //  TRUE,   0x0, mfc120uPatch,
			"mfc120ud.dll",                                                                        // TRUE,   0x0, mfc120udPatch,
			"mfc140.dll",                                                                        //   TRUE,   0x0, mfc140Patch,
			"mfc140d.dll",                                                                        //  TRUE,   0x0, mfc140dPatch,
			"mfc140u.dll",                                                                        //  TRUE,   0x0, mfc140uPatch,
			"mfc140ud.dll",                                                                        // TRUE,   0x0, mfc140udPatch,

			// CRT new operators and heap APIs.
			"msvcrt.dll",                                                                        //   FALSE,  0x0, msvcrtPatch,
			"msvcrtd.dll",                                                                        //  FALSE,  0x0, msvcrtdPatch,
			"msvcr70.dll",                                                                        //  FALSE,  0x0, msvcr70Patch,
			"msvcr70d.dll",                                                                        // FALSE,  0x0, msvcr70dPatch,
			"msvcr71.dll",                                                                        //  FALSE,  0x0, msvcr71Patch,
			"msvcr71d.dll",                                                                        // FALSE,  0x0, msvcr71dPatch,
			"msvcr80.dll",                                                                        //  FALSE,  0x0, msvcr80Patch,
			"msvcr80d.dll",                                                                        // FALSE,  0x0, msvcr80dPatch,
			"msvcr90.dll",                                                                        //  FALSE,  0x0, msvcr90Patch,
			"msvcr90d.dll",                                                                        // FALSE,  0x0, msvcr90dPatch,
			"msvcr100.dll",                                                                        // FALSE,  0x0, msvcr100Patch,
			"msvcr100d.dll",                                                                        //FALSE,  0x0, msvcr100dPatch,
			"msvcr110.dll",                                                                        // FALSE,  0x0, msvcr110Patch,
			"msvcr110d.dll",                                                                        //FALSE,  0x0, msvcr110dPatch,
			"msvcr120.dll",                                                                        // FALSE,  0x0, msvcr120Patch,
			"msvcr120d.dll",                                                                        //FALSE,  0x0, msvcr120dPatch,
			"msvcr140d.dll",                                                                        // FALSE,  0x0, ucrtbasePatch,
			"ucrtbase.dll",                                                                        // FALSE,  0x0, ucrtbasePatch,
			"ucrtbased.dll",                                                                        //FALSE,  0x0, ucrtbasedPatch,

			// NT APIs.
			"ntdll.dll",                                                                        //    FALSE,  0x0, m_ntdllPatch,

			// COM heap APIs.
			"ole32.dll",                                                                        //    FALSE,  0x0, m_ole32Patch
			* /
			"Alice_d.exe",
			"ntdll.dll",
			"kernel32.dll",
			"KernelBase.dll",
			"msvcp140d.dll",
			"vcruntime140d.dll",
			"ucrtbased.dll",
			"api-ms-win-core-timezone-l1-1-0.dll",
			"api-ms-win-core-file-l2-1-0.dll",
			"api-ms-win-core-localization-l1-2-0.dll",
			"api-ms-win-core-synch-l1-2-0.dll",
			"api-ms-win-core-processthreads-l1-1-1.dll",
			"api-ms-win-core-file-l1-2-0.dll",
			"advapi32.dll",
			"msvcrt.dll",
			"sechost.dll",
			"rpcrt4.dll",
			"sspicli.dll",
			"cryptbase.dll",
			"OpenAL32.dll",
			"winmm.dll",
			"user32.dll",
			"gdi32.dll",
			"lpk.dll",
			"usp10.dll",
			"alut.dll",
			"dbghelp.dll",
			"glu32.dll",
			"opengl32.dll",
			"ddraw.dll",
			"dciman32.dll",
			"setupapi.dll",
			"cfgmgr32.dll",
			"oleaut32.dll",
			"ole32.dll",
			"devobj.dll",
			"dwmapi.dll",
			"libfbxsdk.dll",
			"imm32.dll",
			"msctf.dll",
		};
		for (int i =  ARRAYSIZE(m_patchTable); i--;) {
			auto m_patchTableStr = m_patchTable[i];
				msvcrXd = GetModuleHandleA(m_patchTable[i]);
				if (msvcrXd != null) {
					//*
					FARPROC proc[] = { 
						GetProcAddress(msvcrXd, "HeapAlloc"),
						GetProcAddress(msvcrXd, "??2@YAPAXI@Z"),			// new(uint)
						GetProcAddress(msvcrXd, "??2@YAPAXIHPBDH@Z"),		// new(uint, const char*, int)
						GetProcAddress(msvcrXd, "??3@YAXPAX@Z"),			// delete(void*);
						GetProcAddress(msvcrXd, "??_U@YAPAXI@Z"),			// new [] (uint)
						GetProcAddress(msvcrXd, "??_U@YAPAXIHPBDH@Z"),	// new [] (uint, const char*, int)
						GetProcAddress(msvcrXd, "??_V@YAXPAX@Z"),			// delete [] (void*);
						GetProcAddress(msvcrXd, "malloc"),				//
						GetProcAddress(msvcrXd, "_malloc_base"),			//
						GetProcAddress(msvcrXd, "_malloc_dbg"),			//
						GetProcAddress(msvcrXd, "_calloc_dbg"),			//
						GetProcAddress(msvcrXd, "calloc"),				//
						GetProcAddress(msvcrXd, "realloc"),				//
						GetProcAddress(msvcrXd, "_realloc_dbg"),			//
						GetProcAddress(msvcrXd, "_free_base"),			//
						GetProcAddress(msvcrXd, "_free_dbg"),				//
						GetProcAddress(msvcrXd, "free"),					//
						0,

						GetProcAddress(msvcrXd, "??2@YAPEAX_K@Z"),	// new(uint)
						GetProcAddress(msvcrXd, "??_U@YAPEAX_K@Z"),	// new [] (uint)0
						GetProcAddress(msvcrXd, "??3@YAXPEAX@Z"),	// delete(void*);
						GetProcAddress(msvcrXd, "??_V@YAXPEAX@Z"),	// delete[](void*);
						GetProcAddress(msvcrXd, "??2@YAPEAX_KPEBDH@Z"),	// new(uint, const char*, int)
						GetProcAddress(msvcrXd, "??_U@YAPEAX_KPEBDH@Z"),	// new [] (uint, const char*, int)

					};// * /

					if( proc[0] != null )
						continue;;
					for (int j = ARRAYSIZE(proc); j--; ) {
						if (proc[j] != null)
							break;

					}
				} else 
					continue;;
		}
		msvcrXd = GetModuleHandleA(msvcdll.str());
		/*
		"??2@YAPAXI@Z"),			// new(uint)
		"??2@YAPAXIHPBDH@Z"),		// new(uint, const char*, int)
		"??3@YAXPAX@Z"),			// delete(void*);
		"??_U@YAPAXI@Z"),			// new [] (uint)
		"??_U@YAPAXIHPBDH@Z"),	// new [] (uint, const char*, int)
		"??_V@YAXPAX@Z"),			// delete [] (void*);


		x64

		void * __cdecl operator new(unsigned __int64)" (??2@YAPEAX_K@Z)
		void __cdecl operator delete(void *)" (??3@YAXPEAX@Z)
		void __cdecl operator delete[](void *)" (??_V@YAXPEAX@Z)
		void * __cdecl operator new[](unsigned __int64)" (??_U@YAPEAX_K@Z)


		void * __cdecl operator new(unsigned __int64,char const *,unsigned int)" (??2@YAPEAX_KPEBDI@Z)
		void * __cdecl operator new[](unsigned __int64,char const *,unsigned int)" (??_U@YAPEAX_KPEBDI@Z)

		void * __cdecl operator new(unsigned __int64,char const *,unsigned __int64)" (??2@YAPEAX_KPEBD0@Z)
		void * __cdecl operator new[](unsigned __int64,char const *,unsigned __int64)" (??_U@YAPEAX_KPEBD0@Z)

		void * __cdecl operator new(unsigned __int64,char const *,int)" (??2@YAPEAX_KPEBDH@Z)
		void * __cdecl operator new[](unsigned __int64,char const *,int)" (??_U@YAPEAX_KPEBDH@Z)


		void* operator new ( size_t a ) { return 0; }
		void* operator new ( size_t a, const char *c, int i ) { return 0; }
		void* operator new[] ( size_t a ) { return 0; }
		void* operator new[] ( size_t a, const char *c, int i ) { return 0; }
		void operator delete( void*) {}
		void operator delete[]( void*) {}
		* /
	
	//*			

		OrigHeapAlloc = (OrigHeapAlloc_T)GetProcAddress(GetModuleHandleA("kernel32.dll"), "HeapAlloc");
		for (int i = ARRAYSIZE(m_patchTable); i--;) {
			auto m_patchTableStr = m_patchTable[i];
			break;
			ULONG size;
			//    EnterCriticalSection(&imagelock); //todo multithread
			auto clientMod = GetModuleHandleA(m_patchTable[i]);
			auto oldProc = GetProcAddress(GetModuleHandleA("kernel32.dll"), "HeapAlloc");
			IMAGE_IMPORT_DESCRIPTOR *iid = (IMAGE_IMPORT_DESCRIPTOR*)ImageDirectoryEntryToDataEx(clientMod, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size, NULL);
			// LeaveCriticalSection(&imagelock);
			if (iid == NULL) 
				continue;; //Failed  - could not not find a table refering to clientMod

			for (;; iid++) {
				if (!iid->OriginalFirstThunk)										break; // Failed! - to find relavent reference table 	
																						   //if (isEqual(serviceMod, (char*)offsetPtr(clientMod, iid->Name)))	break; // these are droids you are looking for

				for (IMAGE_THUNK_DATA *itd = (IMAGE_THUNK_DATA*)offsetPtr(clientMod, iid->FirstThunk); itd->u1.Function != 0x0; itd++) {
					auto fp = (FARPROC)itd->u1.Function;
					if (itd->u1.Function == (size_t)oldProc) {
						DWORD oProtect;
						VirtualProtect(&itd->u1.Function, sizeof(itd->u1.Function), PAGE_READWRITE, &oProtect); //ensure we are allowed to overwrite table entry
						itd->u1.Function = (size_t)_my_HeapAlloc;
						VirtualProtect(&itd->u1.Function, sizeof(itd->u1.Function), oProtect, &oProtect);
					}
				}

			}

		}
		ULONG size;
		//    EnterCriticalSection(&imagelock); //todo multithread
		auto clientMod =mod;
		auto oldProc = GetProcAddress(GetModuleHandleA("kernel32.dll"), "HeapAlloc" );
		IMAGE_IMPORT_DESCRIPTOR *iid = (IMAGE_IMPORT_DESCRIPTOR*)ImageDirectoryEntryToDataEx(clientMod, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size, NULL);
		// LeaveCriticalSection(&imagelock);
		if (iid != NULL)

			for (;; iid++) {
				if (!iid->OriginalFirstThunk)										break; // Failed! - to find relavent reference table 	
																						   //if (isEqual(serviceMod, (char*)offsetPtr(clientMod, iid->Name)))	break; // these are droids you are looking for
				break;
				for (IMAGE_THUNK_DATA *itd = (IMAGE_THUNK_DATA*)offsetPtr(clientMod, iid->FirstThunk); itd->u1.Function != 0x0; itd++) {
					auto fp = (FARPROC)itd->u1.Function;
					if (itd->u1.Function == (size_t)oldProc) {
						//	WrapProc<AllocWrapper<2>>(false, clientMod, GetModuleHandleA("msvcrt.dll"), CSTR("msvcrt.dll"), CSTR(operatorSymbols[0])); // new(uint)
						//WrapProc<AllocWrapper<3>>(false, clientMod, GetModuleHandleA("msvcrt.dll"), CSTR("msvcrt.dll"), CSTR(operatorSymbols[1]));
						DWORD oProtect;
						VirtualProtect(&itd->u1.Function, sizeof(itd->u1.Function), PAGE_READWRITE, &oProtect); //ensure we are allowed to overwrite table entry
						itd->u1.Function = (size_t) _my_HeapAlloc;
						VirtualProtect(&itd->u1.Function, sizeof(itd->u1.Function), oProtect, &oProtect);
						break;
					}
				}

			}

		auto d = HeapAlloc( GetProcessHeap(), 0, 1);
		//*/
		WrapProc<AllocWrapper<0>>		( mod, msvcrXd, msvcdll, CSTR("malloc")			);
	
	//	WrapProc<AllocWrapper<1>>		( mod, msvcrXd, msvcdll, CSTR("_malloc_base")		);
		//WrapProc<AllocWrapper<2>>		( mod, GetModuleHandleA("msvcrt.dll"), CSTR("msvcrt.dll"), CSTR(operatorSymbols[0])		); // new(uint)
		//WrapProc<AllocWrapper<3>>		( mod, GetModuleHandleA("msvcrt.dll"), CSTR("msvcrt.dll"), CSTR(operatorSymbols[1])		); // new [] (uint)0
		WrapProc<AllocWrapper<2>>		( mod, msvcrXd, msvcdll, CSTR(operatorSymbols[0]), false); // new(uint)
		WrapProc<AllocWrapper<3>>		( mod, msvcrXd, msvcdll, CSTR(operatorSymbols[1]), false); // new [] (uint)0

		WrapProc<ReallocWrapper<0>>		( mod, msvcrXd, msvcdll, CSTR("realloc")			);
		WrapProc<FreeWrapper<0>>		( mod, msvcrXd, msvcdll, CSTR("free")				);
		//WrapProc<FreeWrapper<1>>		( mod, msvcrXd, msvcdll, CSTR("_free_base")		);
		WrapProc<FreeDbgWrapper<0>>		( mod, msvcrXd, msvcdll, CSTR("_free_dbg")		);
		WrapProc<AllocDbgWrapper<0>>	( mod, msvcrXd, msvcdll, CSTR("_malloc_dbg")		);
		WrapProc<AllocDbgWrapper<1>>	( mod, msvcrXd, msvcdll, CSTR(operatorSymbols[4]), false	); // new(uint, const char*, int)
		WrapProc<AllocDbgWrapper<2>>	( mod, msvcrXd, msvcdll, CSTR(operatorSymbols[5]), false	); // new [] (uint, const char*, int)
	
		WrapProc<FreeWrapper<2>>(mod, msvcrXd, msvcdll, CSTR(operatorSymbols[2]), false); // delete(void*);
		WrapProc<FreeWrapper<3>>(mod, msvcrXd, msvcdll, CSTR(operatorSymbols[3]), false); // delete [] (void*);

		for (int i = 100; i--; ) {
			
			auto t = new int;
			delete t;
			auto t2 = new int [300 - i];
			delete[] t2;

			auto t3 = malloc(3 * i);
			free(t3);
		}
		int a = 0;
	}

	void activateForModule( const char *name ) {
		HMODULE mod = null;
		if (name != null) mod = GetModuleHandleA(name);
		activateForModule( mod  );  
	}
	void deactivateForModule( const char *name );

	void printAll(bool unloadSymbolsAfter) { HeapBlock::printAll(unloadSymbolsAfter);  }
}	}

#endif //DEBUG