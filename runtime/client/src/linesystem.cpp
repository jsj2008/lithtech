
#include "bdefs.h"
#include "de_objects.h"
#include "linesystem.h"
#include "clientmgr.h"



// -------------------------------------------------------------------- //
// Internals.
// -------------------------------------------------------------------- //

inline void linesystem_CopyLTLineToLSLine(LTLine *pIn, LSLine *pFillIn)
{
	int i;

	for(i=0; i < 2; i++)
	{
		pFillIn->m_Points[i].m_Pos = pIn->m_Points[i].m_Pos;
		
		pFillIn->m_Points[i].r = pIn->m_Points[i].r;
		pFillIn->m_Points[i].g = pIn->m_Points[i].g;
		pFillIn->m_Points[i].b = pIn->m_Points[i].b;
		pFillIn->m_Points[i].a = pIn->m_Points[i].a;
	}
}


inline void linesystem_CopyLSLineToLTLine(LSLine *pIn, LTLine *pFillIn)
{
	int i;

	for(i=0; i < 2; i++)
	{
		pFillIn->m_Points[i].m_Pos = pIn->m_Points[i].m_Pos;
		
		pFillIn->m_Points[i].r = pIn->m_Points[i].r;
		pFillIn->m_Points[i].g = pIn->m_Points[i].g;
		pFillIn->m_Points[i].b = pIn->m_Points[i].b;
		pFillIn->m_Points[i].a = pIn->m_Points[i].a;
	}
}


inline void linesystem_ExtendBounds(LineSystem *pSystem, LTVector *pPt)
{
	VEC_MIN(pSystem->m_MinPos, pSystem->m_MinPos, *pPt);

	VEC_MAX(pSystem->m_MaxPos, pSystem->m_MaxPos, *pPt);
}


inline void linesystem_CalcExtents(LineSystem *pSystem)
{
	LTVector half;

	half = pSystem->m_MaxPos - pSystem->m_MinPos;
	half *= 0.5f;
	pSystem->m_SystemCenter = pSystem->m_MinPos + half;
	pSystem->m_SystemRadius = half.Mag() + 1.0f;
}


// -------------------------------------------------------------------- //
// External functions.
// -------------------------------------------------------------------- //

HLTLINE linesystem_GetNextLine(HLOCALOBJ hObj, HLTLINE hPrev)
{
	LineSystem *pSystem;
	LSLine *pLine;
	
	pSystem = (LineSystem*)hObj;
	if(!pSystem || pSystem->m_ObjectType != OT_LINESYSTEM)
		return LTNULL;

	if(hPrev)
	{
		pLine = (LSLine*)hPrev;

		if(pLine->m_pNext == &pSystem->m_LineHead)
			return LTNULL;

		return (HLTLINE)pLine->m_pNext;
	}
	else
	{
		if(pSystem->m_LineHead.m_pNext == &pSystem->m_LineHead)
			return LTNULL;

		return (HLTLINE)pSystem->m_LineHead.m_pNext;
	}
}


void linesystem_GetLineInfo(HLTLINE hLine, LTLine *pFillIn)
{
	LSLine *pLine;

	if(!hLine || !pFillIn)
		return;
	
	pLine = (LSLine*)hLine;
	linesystem_CopyLSLineToLTLine(pLine, pFillIn);	
}


void linesystem_SetLineInfo(HLTLINE hLine, LTLine *pInput)
{
	LSLine *pLine;

	if(!hLine || !pInput)
		return;
	
	pLine = (LSLine*)hLine;
	
	linesystem_CopyLTLineToLSLine(pInput, pLine);
	
	linesystem_ExtendBounds(pLine->m_pSystem, &pLine->m_Points[0].m_Pos);
	linesystem_ExtendBounds(pLine->m_pSystem, &pLine->m_Points[1].m_Pos);
	linesystem_CalcExtents(pLine->m_pSystem);

	pLine->m_pSystem->m_bChanged = LTTRUE;
}


HLTLINE linesystem_AddLine(HLOCALOBJ hObj, LTLine *pInput)
{
	LSLine *pLine;
	LineSystem *pSystem;

	pSystem = (LineSystem*)hObj;
	if(!pSystem || !pInput || pSystem->m_ObjectType != OT_LINESYSTEM)
		return LTNULL;

	pLine = (LSLine*)sb_Allocate(pSystem->m_pLineBank);
	pLine->m_pNext = &pSystem->m_LineHead;
	pLine->m_pPrev = pSystem->m_LineHead.m_pPrev;
	pLine->m_pPrev->m_pNext = pLine->m_pNext->m_pPrev = pLine;
	pLine->m_pSystem = pSystem;

	linesystem_CopyLTLineToLSLine(pInput, pLine);
	linesystem_ExtendBounds(pSystem, &pLine->m_Points[0].m_Pos);
	linesystem_ExtendBounds(pSystem, &pLine->m_Points[1].m_Pos);
	linesystem_CalcExtents(pSystem);

	pSystem->m_bChanged = LTTRUE;
	return (HLTLINE)pLine;
}


void linesystem_RemoveLine(HLOCALOBJ hObj, HLTLINE hLine)
{
	LSLine *pLine;
	LineSystem *pSystem;

	if(!hObj || !hLine)
		return;

	pLine = (LSLine*)hLine;
	pSystem = ToLineSystem((LTObject*)hObj);
	if(pSystem->m_ObjectType != OT_LINESYSTEM)
		return;

	pLine->m_pNext->m_pPrev = pLine->m_pPrev;
	pLine->m_pPrev->m_pNext = pLine->m_pNext;
	sb_Free(pSystem->m_pLineBank, pLine);
}








