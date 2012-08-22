
#include "stdafx.h"
#include "bdefs.h"
#include "ltmem.h"
#include "ltbasedefs.h"
#include "string.h"
#include "ltmemheap.h"
#include "ltmemtrack.h"


///////////////////////////////////////////////////////////////////////////////////////////
// information about the current allocation
///////////////////////////////////////////////////////////////////////////////////////////
CMemTrackAllocInfo g_curAllocInfo;


///////////////////////////////////////////////////////////////////////////////////////////
// information about the current allocation information depth
// if this is 0 then the information is not valid
///////////////////////////////////////////////////////////////////////////////////////////
uint32 g_nCurMemTrackAllocInfoDepth = 0;


///////////////////////////////////////////////////////////////////////////////////////////
// number of calls to alloc counter
///////////////////////////////////////////////////////////////////////////////////////////
uint32 g_nMemTrackAllocationCount = 0;


///////////////////////////////////////////////////////////////////////////////////////////
// number of calls to free counter
///////////////////////////////////////////////////////////////////////////////////////////
uint32 g_nMemTrackFreeCount = 0;


///////////////////////////////////////////////////////////////////////////////////////////
// list to hold translation information for strings to data types
///////////////////////////////////////////////////////////////////////////////////////////
CMemTrackTypeToString* g_pLTMemTrackTypeToStringList = NULL;


///////////////////////////////////////////////////////////////////////////////////////////
// current total amount of memory allocated based on memory requested
///////////////////////////////////////////////////////////////////////////////////////////
uint32 g_nMemTrackTotalAllocated = 0;

///////////////////////////////////////////////////////////////////////////////////////////
// the maximum amount of memory we have had allocated over the course of this run
///////////////////////////////////////////////////////////////////////////////////////////
uint32 g_nMemTrackPeakMemAllocated = 0;

///////////////////////////////////////////////////////////////////////////////////////////
// the maximum number of allocations we have had at once during this run
///////////////////////////////////////////////////////////////////////////////////////////
uint32 g_nMemTrackPeakAllocations = 0;

///////////////////////////////////////////////////////////////////////////////////////////
// pointer to list of all track information headers for all memory allocated
///////////////////////////////////////////////////////////////////////////////////////////
CMemTrackAllocInfo* g_pTrackHeap = NULL;

///////////////////////////////////////////////////////////////////////////////////////////
// number of allocations for each type of memory
///////////////////////////////////////////////////////////////////////////////////////////
uint32 g_nMemTypeTrackAllocationCount[LT_NUM_MEM_TYPES];


///////////////////////////////////////////////////////////////////////////////////////////
// init mem tracking	
///////////////////////////////////////////////////////////////////////////////////////////
void LTMemTrackInit()
{
}


///////////////////////////////////////////////////////////////////////////////////////////
// term mem tracking	
///////////////////////////////////////////////////////////////////////////////////////////
void LTMemTrackTerm()
{
}

///////////////////////////////////////////////////////////////////////////////////////////
// function to add mem type to string translation	
///////////////////////////////////////////////////////////////////////////////////////////
void LTMemTrackAddTypeToString(uint32 nType, const char* sName)
{
	if (sName == NULL) 
		return;
	
	CMemTrackTypeToString* pNew;
	LT_MEM_TRACK_ALLOC(pNew = new CMemTrackTypeToString, LT_MEM_TYPE_MEM);

	pNew->m_nType = nType;
	LT_MEM_TRACK_ALLOC(pNew->m_sName = new char[strlen(sName)+1], LT_MEM_TYPE_MEM);

	strcpy(pNew->m_sName, sName);

	pNew->m_pNext = g_pLTMemTrackTypeToStringList;
	g_pLTMemTrackTypeToStringList = pNew;
}


///////////////////////////////////////////////////////////////////////////////////////////
// function to get mem ptr to type from type	
///////////////////////////////////////////////////////////////////////////////////////////
CMemTrackTypeToString* LTMemTrackGetPointerFromType(uint32 nType)
{
	CMemTrackTypeToString* pGet = g_pLTMemTrackTypeToStringList;
	while (pGet != NULL)
	{
		if (pGet->m_nType == nType) 
			return pGet;
		else 
			pGet = pGet->m_pNext;
	}
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////////////////
// engine memory types to strings translation setup	
///////////////////////////////////////////////////////////////////////////////////////////
void LTMemTrackSetupMemTypesToStrings()
{
	LTMemTrackAddTypeToString(LT_MEM_TYPE_UNKNOWN, "unknown");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_MISC, "misc");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_TEXTURE, "texture");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_MODEL, "model");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_SPRITE, "sprite");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_SOUND, "sound");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_OBJECT, "object");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_WORLD, "world");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_HEIGHTMAP, "heightmap");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_PCX, "pcx");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_MUSIC, "music");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_FILE, "file");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_UI, "ui");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_MEM, "mem");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_STRING, "string");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_HASHTABLE, "hashtable");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_WORLDTREE, "worldtree");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_NETWORKING, "networking");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_RENDERER, "renderer");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_RENDER_SHADER, "render shader");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_RENDER_WORLD, "render world");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_RENDER_LIGHTMAP, "render lightmap");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_RENDER_LIGHTGROUP, "render lightgroup");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_RENDER_TEXTURESCRIPT, "render texturescript");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_CONSOLE, "console");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_INTERFACEDB, "interface db");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_INPUT, "input");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_PROPERTY, "property");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_CLIENTSHELL, "ClientShell");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_OBJECTSHELL, "ObjectShell");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_CLIENTFX, "ClientFX");
	LTMemTrackAddTypeToString(LT_MEM_TYPE_GAMECODE, "GameCode");
}


///////////////////////////////////////////////////////////////////////////////////////////
// memory tracking functions that are called from the macros
///////////////////////////////////////////////////////////////////////////////////////////
void LTMemTrackAllocStart(unsigned int nLineNum, const char* sFileName, unsigned int ltAllocType)
{
	g_nCurMemTrackAllocInfoDepth++;

	ASSERT(g_nCurMemTrackAllocInfoDepth);

	if (g_nCurMemTrackAllocInfoDepth > 1) return;

	g_curAllocInfo.m_nLineNum = nLineNum;
	if (sFileName == NULL) 
		g_curAllocInfo.m_sFileName[0] = '\0';
	else
	{
		size_t nStrLen = strlen(sFileName);
		size_t nOffset = 0;
		if (nStrLen >= CMEMTRACKFILENAMESIZE) nOffset = nStrLen - CMEMTRACKFILENAMESIZE + 1;
		strncpy(g_curAllocInfo.m_sFileName, sFileName + nOffset, CMEMTRACKFILENAMESIZE);
		g_curAllocInfo.m_sFileName[CMEMTRACKFILENAMESIZE-1] = '\0';
	}
	g_curAllocInfo.m_nAllocationType = ltAllocType;
}

void LTMemTrackAllocEnd()
{
	ASSERT(g_nCurMemTrackAllocInfoDepth);
	g_nCurMemTrackAllocInfoDepth--;
}

///////////////////////////////////////////////////////////////////////////////////////////
// memory tracking functions that are called from the memory functions
///////////////////////////////////////////////////////////////////////////////////////////

// allocate memory
void* LTMemTrackAlloc(uint32 nRequestedSize)
{
	// memory we have allocated
	uint8* pMem;

	// adjust size to allocate
	uint32 nAdjustedSize = nRequestedSize + sizeof(CMemTrackAllocInfo);

	// allocate memory
	pMem = (uint8*)LTMemHeapAlloc(nAdjustedSize);

	// make sure allocation succeeded
	if (pMem == NULL) return NULL;

	// increment number of allocation counter
	g_nMemTrackAllocationCount++;

	// increment total allocated counter
	g_nMemTrackTotalAllocated += (LTMemHeapGetSize(pMem) - sizeof(CMemTrackAllocInfo));

	//handle updating our peak values
	if(g_nMemTrackTotalAllocated > g_nMemTrackPeakMemAllocated)
		g_nMemTrackPeakMemAllocated = g_nMemTrackTotalAllocated;

	if(g_nMemTrackAllocationCount - g_nMemTrackFreeCount > g_nMemTrackPeakAllocations)
		g_nMemTrackPeakAllocations = g_nMemTrackAllocationCount - g_nMemTrackFreeCount;

	// increment the category allocation counter
	if(g_curAllocInfo.m_nAllocationType < LT_NUM_MEM_TYPES)
		g_nMemTypeTrackAllocationCount[g_curAllocInfo.m_nAllocationType]++;

	// make sure this allocation information is valid if not fill in values
	if (g_nCurMemTrackAllocInfoDepth == 0)
	{
		g_curAllocInfo.m_nLineNum = 0;
		g_curAllocInfo.m_sFileName[0] = '\0';
		g_curAllocInfo.m_nAllocationType = LT_MEM_TYPE_UNKNOWN;
	}

	// fill in additional allocation information
	g_curAllocInfo.m_nAllocCount = g_nMemTrackAllocationCount;
	g_curAllocInfo.m_nRequestedSize = nRequestedSize;
	g_curAllocInfo.m_nActualSize = LTMemHeapGetSize(pMem); 
	g_curAllocInfo.m_pMem = (void*)pMem;

	// copy data to start of our memory
	memcpy(pMem, &g_curAllocInfo, sizeof(CMemTrackAllocInfo));

	// insert header into our tracking list
	if (g_pTrackHeap != NULL) 
		g_pTrackHeap->m_pPrev = (CMemTrackAllocInfo*)pMem;

	((CMemTrackAllocInfo*)pMem)->m_pPrev = NULL;
	((CMemTrackAllocInfo*)pMem)->m_pNext = g_pTrackHeap;

	g_pTrackHeap = (CMemTrackAllocInfo*)pMem;

	// adjust mem pointer to point after our header
	pMem += sizeof(CMemTrackAllocInfo);

	//The following lines of code can be used to identify blocks of memory that aren't
	//setup to track properly. Just uncomment them and set a break point on the
	//variable and it will get hit whenever there is untracked memory.
	/*
	if(g_curAllocInfo.m_nAllocationType == LT_MEM_TYPE_UNKNOWN)
	{
		uint32 nBreakOnUnknownMem = 1;
	}
	*/

	// return allocated pointer
	return pMem;
}


// free memory
void LTMemTrackFree(void* pMem)
{
	// adjust pointer to actual position
	uint8* pMemActual = (uint8*)pMem;
	pMemActual -= sizeof(CMemTrackAllocInfo);

	// pointer to header
	CMemTrackAllocInfo* pInfo = (CMemTrackAllocInfo*)pMemActual;

	// decrement total allocated counter
	g_nMemTrackTotalAllocated -= (LTMemHeapGetSize(pMemActual) - sizeof(CMemTrackAllocInfo));

	// increment the total number of free's counter
	g_nMemTrackFreeCount++;

	// remove header from our tracking list
	if (pInfo->m_pNext != NULL) pInfo->m_pNext->m_pPrev = pInfo->m_pPrev;
	if (pInfo->m_pPrev != NULL) pInfo->m_pPrev->m_pNext = pInfo->m_pNext;
	else g_pTrackHeap = pInfo->m_pNext;

	// free the memory
	LTMemHeapFree(pMemActual); 
}


// re-allocate memory
void* LTMemTrackReAlloc(void* pMemOld, uint32 nRequestedSize)
{
	// adjust pointer to actual position
	uint8* pMemActual = (uint8*)pMemOld;
	pMemActual -= sizeof(CMemTrackAllocInfo);

	// pointer to header
	CMemTrackAllocInfo* pInfo = (CMemTrackAllocInfo*)pMemActual;

	// decrement size by old amount
	g_nMemTrackTotalAllocated -= (LTMemHeapGetSize(pMemActual) - sizeof(CMemTrackAllocInfo));

	// adjust size to allocate
	uint32 nAdjustedSize = nRequestedSize + sizeof(CMemTrackAllocInfo);

	// allocate memory
	uint8* pMem = (uint8*)LTMemHeapReAlloc(pMemActual, nAdjustedSize);

	// make sure allocation succeeded
	if (pMem == NULL) return NULL;

	// increment number of allocations and frees counter
	g_nMemTrackAllocationCount++;
	g_nMemTrackFreeCount++;

	// increment total allocated counter by new size
	g_nMemTrackTotalAllocated += (LTMemHeapGetSize(pMem) - sizeof(CMemTrackAllocInfo));

	// make sure this allocation information is valid if not fill in values
	if (g_nCurMemTrackAllocInfoDepth == 0)
	{
		g_curAllocInfo.m_nLineNum = 0;
		g_curAllocInfo.m_sFileName[0] = '\0';
		g_curAllocInfo.m_nAllocationType = 0;
	}

	// fill in additional allocation information
	g_curAllocInfo.m_nAllocCount = g_nMemTrackAllocationCount;
	g_curAllocInfo.m_nRequestedSize = nRequestedSize;
	g_curAllocInfo.m_nActualSize = LTMemHeapGetSize(pMem); 
	g_curAllocInfo.m_pMem = (void*)pMem;
	g_curAllocInfo.m_pNext = pInfo->m_pNext;
	g_curAllocInfo.m_pPrev = pInfo->m_pPrev;

	// copy data to start of our memory
	memcpy(pMem, &g_curAllocInfo, sizeof(CMemTrackAllocInfo));

	// fixup pointers in our tracking list
	if (g_curAllocInfo.m_pNext != NULL) g_curAllocInfo.m_pNext->m_pPrev = (CMemTrackAllocInfo*)pMem;
	if (g_curAllocInfo.m_pPrev != NULL) g_curAllocInfo.m_pPrev->m_pNext = (CMemTrackAllocInfo*)pMem;
	if (g_pTrackHeap == pInfo) g_pTrackHeap = (CMemTrackAllocInfo*)pMem;
	
	// adjust mem pointer to point after our header
	pMem += sizeof(CMemTrackAllocInfo);

	// return allocated pointer
	return pMem;
}


// get size
uint32 LTMemTrackGetSize(void* pMem)
{
	// pointer to header
	CMemTrackAllocInfo* pInfo = (CMemTrackAllocInfo*)pMem;

	// return the actual size
	return pInfo->m_nActualSize;
}




