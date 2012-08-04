/****************************************************************************
;
;	MODULE:		LTRMap (.H)
;
;	PURPOSE:	Support class for RealVideo
;
;	HISTORY:	5-12-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRMap_H
#define LTRMap_H

//#define LITHTECH_ESD_INC 1
//#include "lith.h"
#include "bdefs.h"
//#undef LITHTECH_ESD_INC

//-----------------------------------------------------------------------------
class LTRMap
{
    /****** Class Variables ***********************************************/
    const int AllocationSize;
    void**  m_pKeyArray;
    void**  m_pValueArray;
    int	    m_nMapSize;
    int	    m_nAllocSize;
    int     m_nCursor;

    public:
    /****** Public Class Methods ******************************************/
    LTRMap()
	: m_pKeyArray(NULL)
	, m_pValueArray(NULL)
	, m_nMapSize(0)
	, m_nAllocSize(0)
        , m_nCursor(0)
	, AllocationSize(10)
	{};

    ~LTRMap()
	{
	    delete [] m_pKeyArray;
	    delete [] m_pValueArray;
	};

    int   GetCount() {return m_nMapSize;}
    void* GetFirstValue();
    void* GetNextValue();
    BOOL  Lookup(void* Key, void*& Value) const;
    void  RemoveKey(void* Key);
    void  RemoveValue(void* Value);
    void  SetAt(void* Key, void* Value);
};

#endif // LTRMap_H
#endif // LITHTECH_ESD