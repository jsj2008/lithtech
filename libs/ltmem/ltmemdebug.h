
#ifndef __LTMEMDEBUG_H__
#define __LTMEMDEBUG_H__

void LTMemDebugInit();

void LTMemDebugTerm();

void* LTMemDebugAlloc(uint32 nRequestedSize);

void LTMemDebugFree(void* pMem);

void* LTMemDebugReAlloc(void* pMemOld, uint32 nRequestedSize);

uint32 LTMemDebugGetSize(void* pMem);

#endif
