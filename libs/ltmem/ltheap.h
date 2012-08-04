
// macros to track allocations

LT_MEM_TRACK_ALLOC(expression,location,datatype)

LT_MEM_TRACK_FREE(expression)

LT_MEM_TRACK_REALLOC(expression)

LT_FREE_GAME_CLIENT()

// allocation flags and types 
#define LT_MEM_LOCATION_UNKNOWN	0x00000000
#define LT_MEM_LOCATION_ENGINE	0x00000001
#define LT_MEM_LOCATION_GAME	0x00000002

#define LT_MEM_TYPE_TEXTURE		0x00000001
#define LT_MEM_TYPE_MODEL		0x00000002
#define LT_MEM_TYPE_SPRITE		0x00000003
#define LT_MEM_TYPE_SOUND		0x00000004
#define LT_MEM_TYPE_OBJECT		0x00000005
#define LT_MEM_TYPE_WORLD		0x00000006


struct LTHeapSimpleHeapInfo
{
	uint32 m_nElemSize;
	uint32 m_nMaxElements;
}

class LTHeapSimpleHeapListElem
{
	
}

class JupiterHeapSimpleHeapList
{
	
}

struct JupiterHeapInfo
{
}