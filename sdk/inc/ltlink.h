/*!  This is for simple but very convenient doubly linked lists.  You
need an \b LTLink for the list head, and this header merely defines common
routines to insert, remove, and tie off.  */

#ifndef __LTLINK_H__
#define __LTLINK_H__


class LTLink;


#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif




/*!
This goes through and C++ deletes everything in a list.
You need an LTLink *pCur, *pNext;
*/
#define CPP_DELETELIST(_list_pointer, type) \
{\
    pCur = (_list_pointer)->m_Head.m_pNext;\
    while (pCur != &(_list_pointer)->m_Head)\
    {\
        pNext = pCur->m_pNext;\
        delete ((type*)pCur->m_pData);\
        pCur = pNext;\
    }\
    dl_InitList(_list_pointer);\
}




/*!
Use these to declare blank \b LTLinks and \b LTLists.
*/
#define DECLARE_LTLINK(name) \
    LTLink name(LTLink_Init);

#define DECLARE_LTLIST(name) \
    LTList name(LTLink_Init);


typedef enum
{
    LTLink_Init=0
} LTLinkCommand;


#if !defined(force_inline)
#if defined(_MSC_VER) && (_MSC_VER >= 1200) 
  // MS Visual C++ version 6.0 and later
  #define force_inline __forceinline
#else
  #define force_inline inline
#endif
#define local_force_inline
#endif // !defined(force_inline)

/*!  Use \b LTLink whenever you need \b m_pData, but you can use this if you
have a list head.  */

 
class CheapLTLink {
public:
    
    force_inline void Init();
    force_inline void Term();
    force_inline void Remove();
    force_inline void TieOff();
    force_inline void AddAfter(CheapLTLink *pLink);
    force_inline void AddBefore(CheapLTLink *pLink);
    force_inline LTLink *operator[](unsigned long i);
    force_inline LTLink *AsDLink()   {return (LTLink*)this;}
    force_inline LTLink *AsLTLink()  {return (LTLink*)this;}
    force_inline bool IsTiedOff();


public:

    LTLink *m_pPrev, *m_pNext;
};


class LTLink : public CheapLTLink {
public:

    force_inline LTLink() {}
    force_inline LTLink(LTLinkCommand cmd) {
        TieOff();
    }

    force_inline void Init2(void *pData) {
        TieOff();
        m_pData = pData;
    }


public:
    void *m_pData;
};


force_inline void CheapLTLink::Init() {
    TieOff();
}

force_inline void CheapLTLink::Term() {
    Remove();
}

force_inline void CheapLTLink::Remove() {
    m_pPrev->m_pNext = m_pNext;
    m_pNext->m_pPrev = m_pPrev;
    TieOff();
}

force_inline void CheapLTLink::TieOff() {
    m_pPrev = m_pNext = (LTLink*)this;
}

force_inline void CheapLTLink::AddAfter(CheapLTLink *pLink) {
    pLink->m_pPrev = (LTLink*)this;
    pLink->m_pNext = m_pNext;
    pLink->m_pPrev->m_pNext = pLink->m_pNext->m_pPrev = (LTLink*)pLink;
}

force_inline void CheapLTLink::AddBefore(CheapLTLink *pLink) {
    pLink->m_pPrev = m_pPrev;
    pLink->m_pNext = (LTLink*)this;
    pLink->m_pPrev->m_pNext = pLink->m_pNext->m_pPrev = (LTLink*)pLink;
}

/*!
Make it easy to traverse the list in either direction.  
*/
force_inline LTLink* CheapLTLink::operator[](unsigned long i) {
    return ((LTLink**)this)[i];
}

force_inline bool CheapLTLink::IsTiedOff() {
    return m_pPrev == this && m_pNext == this;
}

force_inline void dl_TieOff(CheapLTLink *pLink) {
    pLink->TieOff();
}

 
force_inline void dl_Insert(CheapLTLink *pAfter, CheapLTLink *pLink) {
    pAfter->AddAfter(pLink);
}


force_inline void dl_Remove(CheapLTLink *pLink) {
    pLink->Remove();
}

/*!  You can use this list structure to wrap up a linked list if you
want to keep a count of the elements.  */

class LTList {
public:

    force_inline LTList() {
        m_Head.TieOff();
        m_nElements = 0;
    }
                    
    force_inline LTList(LTLinkCommand cmd) {
        m_Head.TieOff();
        m_nElements = 0;
    }

 
    unsigned long m_nElements;
    LTLink m_Head;
};


force_inline void dl_InitList(LTList *pList) {
    dl_TieOff(&pList->m_Head);
    pList->m_nElements = 0;
}

force_inline void dl_AddAfter(LTList *pList, LTLink *pAfter, LTLink *pLink, void *pObj) {
    pLink->m_pData = pObj;
    dl_Insert(pAfter, pLink);
    ++pList->m_nElements;
}

force_inline void dl_AddBefore(LTList *pList, LTLink *pBefore, LTLink *pLink, void *pObj) {
    dl_AddAfter(pList, pBefore->m_pPrev, pLink, pObj);
}

force_inline void dl_AddHead(LTList *pList, LTLink *pLink, void *pObj) {
    dl_AddAfter(pList, &pList->m_Head, pLink, pObj);
}

force_inline void dl_AddTail(LTList *pList, LTLink *pLink, void *pObj) {
    dl_AddAfter(pList, pList->m_Head.m_pPrev, pLink, pObj);
}

force_inline void dl_RemoveAt(LTList *pList, LTLink *pLink) {
    dl_Remove(pLink);
    --pList->m_nElements;
}


//
// CLTList (TBD: move this to ltlink.h after completely tested)
//
class CLTList : public LTList
{
public:
    force_inline CLTList(){} // list is automatically initialized by the LTList constructor

    class Iterator {
    public:
        force_inline Iterator() : m_pCur(0) {}
        force_inline Iterator(LTLink * pLink) : m_pCur(pLink) {}
        force_inline Iterator(LTLink& link) : m_pCur(&link) {}
        force_inline LTLink * operator->() { return m_pCur; }
        force_inline operator LTLink *() { return m_pCur; }
        force_inline operator LTLink&() { return *m_pCur; }
        force_inline Iterator& operator++() { m_pCur = m_pCur->m_pNext; return *this; }
        force_inline Iterator operator++(int) { Iterator tmp = *this; m_pCur = m_pCur->m_pNext; return tmp; }
        force_inline Iterator& operator--() { m_pCur = m_pCur->m_pPrev; return *this; }
        force_inline Iterator operator--(int) { Iterator tmp = *this; m_pCur = m_pCur->m_pPrev; return tmp; }
        force_inline Iterator& operator=(LTLink * pLink) { m_pCur = pLink; return *this; }
        force_inline bool operator==(LTLink * pLink) const { return pLink == m_pCur; }
        force_inline bool operator!=(LTLink * pLink) const { return pLink != m_pCur; }
    private:
        LTLink * m_pCur;
    }; // Iterator

    force_inline LTLink * Begin() {// support STL-ish "for(iter=list.Begin(); iter!=list.End(); ++iter)"
        return m_Head.m_pNext;
    }

    force_inline LTLink * End() {
        return &m_Head;
    }

    // add link so that "after" comes after link in the list
    // order afterwards:  x <-> link <-> after <-> y
    // pObj is assigned to the m_pData member item of "link"
    // (the semantics may seem odd it matches dl_AddAfter)
    force_inline void AddAfter(LTLink& after, LTLink& link, void *pObj) { 
        dl_AddAfter(this, &after, &link, pObj); 
    }

    // add link so that "after" comes after it in the list
    // order afterwards:  x <-> link <-> after <-> y
    force_inline void AddAfter(LTLink& after, LTLink& link) { // use if link.m_pData already set
        dl_Insert(&after, &link);
        m_nElements++;
    }

    // add link so that "before" comes before link in the list
    // order afterwards:  x <-> before <-> link <-> y
    // pObj is assigned to the m_pData member item of "link"
    force_inline void AddBefore(LTLink& before, LTLink& link, void * pObj) { 
        dl_AddAfter(this, before.m_pPrev, &link, pObj); 
    }

    // add link so that "before" comes before link in the list
    // order afterwards:  x <-> before <-> link <-> y
    force_inline void AddBefore(LTLink& before, LTLink& link) {// use if link.m_pData already set 
        dl_Insert(before.m_pPrev, &link);
        m_nElements++;
    }

    force_inline void AddHead(LTLink& link, void * pObj) { 
        dl_AddHead(this, &link, pObj); 
    }

    force_inline void AddHead(LTLink& link) {// use if link.m_pData already set
        AddAfter(m_Head, link); 
    }

    force_inline void AddTail(LTLink& link, void * pObj) { 
        dl_AddTail(this, &link, pObj); 
    }

    force_inline void AddTail(LTLink& link) {// use if link.m_pData already set
        AddAfter(*m_Head.m_pPrev, link); 
    }

 
    force_inline uint32 GetCount() const {
        return m_nElements; 
    }


    force_inline void Remove(LTLink& link) { 
        dl_RemoveAt(this, &link); 
    }

    force_inline void Reset() { 
        dl_InitList(this); 
    }
}; // CLTList

#if defined(local_force_inline)
#undef force_inline
#undef local_force_inline
#endif

typedef CLTList::Iterator CLTListIterator;

#endif  //! __LTLINK_H__

