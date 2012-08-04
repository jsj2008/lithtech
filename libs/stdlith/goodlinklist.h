#ifndef __GOODLINKLIST_H__
#define __GOODLINKLIST_H__

#ifndef __STDLITHDEFS_H__
#include "stdlithdefs.h"
#endif

#ifndef __GENLIST_H__
#include "genlist.h"
#endif

class CGLLNode
{
    public:
        
        CGLLNode    *m_pGPrev, *m_pGNext;

};

typedef CGLLNode*   GPOS;


// This is defined as a separate function because it's called by the virtual
// GenFindElement and we don't want to generate a bunch of template code for
// every type of object we use in a list.
GPOS GLinkedList_FindElementMemcmp(
    CGLLNode *pHead,
    const void *pToFind,
    uint32 elementSize);


template<class T>
class CGLinkedList : public GenList<T>
{
protected:
    
        

public:

                CGLinkedList()
                {
                    m_nElements = 0;
                    m_pHead = NULL;
                }

                ~CGLinkedList()
                {
                    RemoveAll();
                }

    inline void Term() {RemoveAll();}

        operator uint32() {return m_nElements;}
        operator GPOS() { return m_pHead; }
    
    uint32      GetCount()  const       { return m_nElements; }
    uint32      GetSize()   const       { return m_nElements; }
    int         IsEmpty()   const       { return !m_pHead; }

    inline T        GetHead()   const;
    inline T        GetTail()   const;

    inline T        RemoveHead();
    inline T        RemoveTail();

    inline void     Append(T newTail)
    {
        AddTail(newTail);
    }

    inline GPOS     AddHead(T newHead);
    inline GPOS     AddTail(T newTail);

    inline void     RemoveAll();

    GPOS        GetHeadPosition()   const   { return m_pHead; }
    GPOS        GetTailPosition()   const   { return (m_pHead ? m_pHead->m_pGPrev : NULL); }

    inline T        GetNext(GPOS &pos)    const;
    inline T        GetPrev(GPOS &pos)    const;
    
    inline T        GetAt(GPOS pos) const;
    
    inline void     RemoveAt(GPOS pos);

    GPOS        InsertBefore(GPOS pos, T el);
    GPOS        InsertAfter(GPOS pos, T el);

    // Find and compare with T::operator==.
    GPOS        Find(T searchFor, uint32 *pIndex=NULL)            const;
    uint32      FindElement(T searchFor, GPOS *pPos=NULL)     const;

    GPOS        FindIndex(uint32 index)                       const;

    // Copy the source list.  This also calls _Term_ on theList.
    void        StealList(CGLinkedList<T> &theList);

    // Yes, we can be a hash bucket.
    static void CheckSupportHashBucket() {}


// GenList implementation.
public:

    virtual GenListPos  GenBegin() const
    {
        #ifdef _DEBUG
            return GenListPos(GetHeadPosition(), 0);
        #else
            return GenListPos(GetHeadPosition());
        #endif
    }

    virtual LTBOOL      GenIsValid(const GenListPos &pos) const
    {
        return !!pos.m_Pos;
    }

    virtual T           GenGetNext(GenListPos &pos) const
    {
        #ifdef _DEBUG
            ++pos.m_Index;
        #endif

        return GetNext(*((GPOS*)&pos.m_Pos));
    }

    virtual T           GenGetAt(GenListPos &pos) const
    {
        return GetAt(*((GPOS*)&pos.m_Pos));
    }

    virtual LTBOOL      GenAppend(T &toAppend)
    {
        Append(toAppend);
        return TRUE;
    }

    virtual void        GenRemoveAt(GenListPos pos)
    {
        RemoveAt((GPOS)pos.m_Pos);
    }

    virtual void        GenRemoveAll()
    {
        RemoveAll();
    }

    virtual uint32      GenGetSize() const
    {
        return GetSize();
    }

    virtual LTBOOL      GenCopyList(const GenList<T> &other)
    {
        RemoveAll();
        return GenAppendList(other);
    }

    virtual LTBOOL      GenAppendList(const GenList<T> &other)
    {
        GenListPos pos;
        
        for (pos=other.GenBegin(); other.GenIsValid(pos);)
        {
            Append(other.GenGetNext(pos));
        }

        return TRUE;
    }

    virtual LTBOOL      GenFindElement(const T &toFind, GenListPos &thePos) const
    {
        thePos.m_Pos = GLinkedList_FindElementMemcmp(
            m_pHead,
            &toFind,
            sizeof(T));

        return !!thePos.m_Pos;
    }


protected:

    uint32          m_nElements;
    CGLLNode        *m_pHead;
};



template<class T>
inline T CGLinkedList<T>::GetHead() const
{
    ASSERT(m_pHead);
    return (T)m_pHead;
}


template<class T>
inline T CGLinkedList<T>::GetTail() const
{
    ASSERT(m_pHead);
    return (T)m_pHead->m_pGPrev;
}


template<class T>
inline T CGLinkedList<T>::GetNext(GPOS &pos) const
{
    T           ret = (T)pos;
    
    if (pos->m_pGNext == m_pHead)
        pos = NULL;
    else
        pos = pos->m_pGNext;

    return ret;
}


template<class T>
inline T CGLinkedList<T>::GetPrev(GPOS &pos) const
{
    T           ret = (T)pos;

    if (pos->m_pGPrev == m_pHead)
        pos = NULL;
    else
        pos = pos->m_pGPrev;

    return ret;
}


template<class T>
inline T CGLinkedList<T>::RemoveHead()
{
    T   ret;

    ASSERT(m_pHead);
    ret = (T)m_pHead;
    RemoveAt(m_pHead);
    
    return ret;
}


template<class T>
inline T CGLinkedList<T>::RemoveTail()
{
    T   ret;

    ASSERT(m_pHead);
    ret = (T)m_pHead->m_pGPrev;
    RemoveAt(m_pHead->m_pGPrev);
    
    return ret;
}


template<class T>
inline GPOS CGLinkedList<T>::AddHead(T newHead)
{
    if (m_pHead)
    {
        return InsertBefore(m_pHead, newHead);
    }
    else
    {
        CGLLNode *pNode = (CGLLNode*)newHead;

        pNode->m_pGNext = (CGLLNode*)newHead;
        pNode->m_pGPrev = (CGLLNode*)newHead;

        m_pHead = (CGLLNode*)newHead;
        ++m_nElements;

        return m_pHead;
    }
}


template<class T>
inline GPOS CGLinkedList<T>::AddTail(T newTail)
{
    if (m_pHead)
        return InsertAfter(m_pHead->m_pGPrev, newTail);
    else
        return AddHead(newTail);
}


template<class T>
inline void CGLinkedList<T>::RemoveAll()
{
    m_pHead = NULL;
    m_nElements = 0;
}


template<class T>
inline T CGLinkedList<T>::GetAt(GPOS pos) const
{
    return (T)pos;
}


template<class T>
void CGLinkedList<T>::RemoveAt(GPOS pos)
{
    ASSERT(m_nElements > 0);
    
    pos->m_pGPrev->m_pGNext = pos->m_pGNext;
    pos->m_pGNext->m_pGPrev = pos->m_pGPrev;
    
    if (pos == m_pHead)
        m_pHead = m_pHead->m_pGNext;
    
    --m_nElements;
    if (m_nElements == 0)
        m_pHead = NULL;
}


template<class T>
GPOS CGLinkedList<T>::InsertBefore(GPOS pos, T el)
{
    pos->m_pGPrev->m_pGNext = (CGLLNode*)el;
    ((CGLLNode*)el)->m_pGPrev = pos->m_pGPrev;
    ((CGLLNode*)el)->m_pGNext = pos;
    pos->m_pGPrev = (CGLLNode*)el;
    
    ++m_nElements;
    if (((CGLLNode*)el)->m_pGNext == m_pHead)
        m_pHead = (CGLLNode*)el;

    return (CGLLNode*)el;
}


template<class T>
GPOS CGLinkedList<T>::InsertAfter(GPOS pos, T el)
{
    pos->m_pGNext->m_pGPrev = (CGLLNode*)el;
    ((CGLLNode*)el)->m_pGNext = pos->m_pGNext;
    ((CGLLNode*)el)->m_pGPrev = pos;
    pos->m_pGNext = (CGLLNode*)el;
    
    ++m_nElements;
    return (CGLLNode*)el;
}


template<class T>
GPOS CGLinkedList<T>::Find(T searchFor, uint32 *pIndex)   const
{
    T           pCur;
    uint32      index = 0;
    
    if (m_pHead)
    {
        pCur = (T)m_pHead;
        do
        {
            
            if (pCur == searchFor)
            {
                if (pIndex)
                    *pIndex = index;

                return pCur;
            }

            pCur = (T)pCur->m_pGNext;
            ++index;

        } while (pCur != (T)m_pHead);
    }

    if (pIndex)
        *pIndex = BAD_INDEX;

    return NULL;
}


template<class T>
uint32 CGLinkedList<T>::FindElement(T searchFor, GPOS *pPos) const
{
    GPOS    pos;
    uint32  index;
    
    pos = Find(searchFor, &index);

    if (pPos)
        *pPos = pos;

    return index;
}


template<class T>
GPOS CGLinkedList<T>::FindIndex(uint32 index) const
{
    T           pCur;
    
    if (index >= m_nElements)
        return NULL;

    pCur = (T)m_pHead;
    do
    {
        
        if (index == 0)
            return pCur;

        pCur = (T)pCur->m_pGNext;
        --index;

    } while (pCur != (T)m_pHead);

    ASSERT(FALSE);    // Shouldn't ever get here.
    return NULL;
}


template<class T>
void CGLinkedList<T>::StealList(CGLinkedList<T> &theList)
{
    m_pHead = theList.m_pHead;
    m_nElements = theList.m_nElements;
    theList.Term();
}



// Useful template function if your linked list is made up of pointers.
template<class T>
void GDeleteAndRemoveElements(CGLinkedList<T> &theList)
{
    GPOS        pos;

    for (pos=theList.GetHeadPosition(); pos != NULL;)
        delete (T)(theList.GetNext(pos));

    theList.RemoveAll();
}


// Useful template function if your linked list is made up of pointers.
template<class T, class OB>
void GDeleteAndRemoveElementsOB(CGLinkedList<T> &theList, OB &objectBank)
{
    GPOS pos;

    for (pos=theList.GetHeadPosition(); pos != NULL;)
    {
        objectBank.Free((T)(theList.GetNext(pos)));
    }

    theList.RemoveAll();
}


#endif // __GOODLINKLIST_H__
    





