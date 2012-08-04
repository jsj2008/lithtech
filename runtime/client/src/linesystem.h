
#ifndef __LINESYSTEM_H__
#define __LINESYSTEM_H__


#ifndef __ILTCLIENT_H__
#include "iltclient.h"
#endif

#ifndef __SETUPOBJECT_H__
#include "setupobject.h"
#endif



HLTLINE linesystem_GetNextLine(HLOCALOBJ hObj, HLTLINE hPrev);
void linesystem_GetLineInfo(HLTLINE hLine, LTLine *pFillIn);
void linesystem_SetLineInfo(HLTLINE hLine, LTLine *pInput);
HLTLINE linesystem_AddLine(HLOCALOBJ hObj, LTLine *pInput);
void linesystem_RemoveLine(HLOCALOBJ hObj, HLTLINE hLine);


#endif  // __LINESYSTEM_H__




