///////////////////////////////////////////////////////////////////////////////////////////
// External functions for use with the LTMem system
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef __LTMEM_H__
#define __LTMEM_H__

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

// uncomment for memory tracking
#ifdef _DEBUG
//#define LTMEMTRACK
#endif

// uncomment for memory debugging
#ifdef _DEBUG
//#define LTMEMDEBUG
#endif

// Initialize LTMem system
void LTMemInit();

// Terminate LTMem system
void LTMemTerm();

// LTMem Allocation function
void* LTMemAlloc(uint32 nSize);

// LTMem Free function
void LTMemFree(void* pMem);

// LTMem system memory re-size function
void* LTMemReAlloc(void* pOldMem, uint32 nNewSize);

// Used to add type to string translation
// you should call this for each different type that are defined
void LTMemTrackAddTypeToString(uint32 nType, const char* sName);


// allocation types
// engine allocations start at 0x00000000 
// if you add a type here you also need to add a type to string translation in the .cpp file
// game allocation types start at 0x00000100

enum
{
	LT_MEM_TYPE_UNKNOWN,
	LT_MEM_TYPE_MISC,
	LT_MEM_TYPE_TEXTURE,
	LT_MEM_TYPE_MODEL,
	LT_MEM_TYPE_SPRITE,
	LT_MEM_TYPE_SOUND,
	LT_MEM_TYPE_OBJECT,
	LT_MEM_TYPE_WORLD,
	LT_MEM_TYPE_HEIGHTMAP,
	LT_MEM_TYPE_PCX,
	LT_MEM_TYPE_MUSIC,
	LT_MEM_TYPE_FILE,
	LT_MEM_TYPE_UI,
	LT_MEM_TYPE_MEM,
	LT_MEM_TYPE_STRING,
	LT_MEM_TYPE_HASHTABLE,
	LT_MEM_TYPE_WORLDTREE,
	LT_MEM_TYPE_NETWORKING,
	LT_MEM_TYPE_RENDERER,
	LT_MEM_TYPE_RENDER_SHADER,
	LT_MEM_TYPE_RENDER_WORLD,
	LT_MEM_TYPE_RENDER_LIGHTMAP,
	LT_MEM_TYPE_RENDER_LIGHTGROUP,
	LT_MEM_TYPE_RENDER_TEXTURESCRIPT,
	LT_MEM_TYPE_CONSOLE,
	LT_MEM_TYPE_INTERFACEDB,
	LT_MEM_TYPE_INPUT,
	LT_MEM_TYPE_PROPERTY,
	LT_MEM_TYPE_CLIENTSHELL,
	LT_MEM_TYPE_OBJECTSHELL,
	LT_MEM_TYPE_CLIENTFX,
	LT_MEM_TYPE_GAMECODE,
	
	//this must come last
	LT_NUM_MEM_TYPES
};

// if mem tracking is on
#ifdef LTMEMTRACK

	// function prototypes for memory tracking functions that are called from the macros
	void LTMemTrackAllocStart(unsigned int nLineNum, const char* sFileName, unsigned int ltAllocType);

	void LTMemTrackAllocEnd();

	// macros to do memory tracking
	#define LT_MEM_TRACK_ALLOC(ltStatement, ltAllocType)	{ LTMemTrackAllocStart(__LINE__, __FILE__, ltAllocType); ltStatement; LTMemTrackAllocEnd(); }

	#define LT_MEM_TRACK_FREE(ltStatement)					{ ltStatement; }

	#define LT_MEM_TRACK_REALLOC(ltStatement, ltAllocType)	{ LTMemTrackAllocStart(__LINE__, __FILE__, ltAllocType); ltStatement; LTMemTrackAllocEnd(); } 

#else

	// macros that do nothing but the basic allocation and free's when mem tracking is off
	#define LT_MEM_TRACK_ALLOC(ltStatement, ltAllocType) ltStatement

	#define LT_MEM_TRACK_FREE(ltStatement) ltStatement

	#define LT_MEM_TRACK_REALLOC(ltStatement, ltAllocType) ltStatement

#endif

#endif // __LTMEM_H__
