/****************************************************************************
;
;	MODULE:		LTRList (.H)
;
;	PURPOSE:	Support class for RealVideo
;
;	HISTORY:	5-12-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRList_H 
#define LTRList_H 

//#define LITHTECH_ESD_INC 1
//#include "lith.h"
#include "bdefs.h"
//#undef LITHTECH_ESD_INC

//-----------------------------------------------------------------------------
class LTRListNode
{
public:
    LTRListNode();
    ~LTRListNode();
    void* m_pData;
    LTRListNode* m_pNext;
};

typedef LTRListNode* LTRListNodePosition;
    
//-----------------------------------------------------------------------------
class LTRList
{
public:
    LTRList();
    ~LTRList();

    void AddHead(void*);
    void Add(void*);
    void* GetFirst();
    void* GetNext();
    void* GetLast();
    void* RemoveHead();
    UINT32 Count() { return m_ulNumElements; }

    LTRListNodePosition GetHeadPosition();
    LTRListNodePosition GetNextPosition(LTRListNodePosition);
    void InsertAfter(LTRListNodePosition, void*);
    void* RemoveAfter(LTRListNodePosition);
    void* GetAt(LTRListNodePosition);

protected:
    LTRListNode* m_pLast;
    LTRListNode* m_pHead;
    LTRListNode* m_pTail;
    UINT32 m_ulNumElements;
};

#endif // LTRList_H 
#endif // LITHTECH_ESD