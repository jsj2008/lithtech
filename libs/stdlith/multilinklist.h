//------------------------------------------------------------------
//
//  FILE      : MultiLinkList.h
//
//  PURPOSE   : Defines the CMultiLinkList class.  This is a linked list
//              that allows more control over its use, so an object can
//              be contained in multiple linked lists.
//
//  CREATED   : December 1 1996
//
//  COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __MULTILINKLIST_H__
#define __MULTILINKLIST_H__


#ifndef ASSERT
    #define ASSERT
#endif

class CMLLNode
{
    public:
        
        CMLLNode    *m_pGPrev, *m_pGNext;
        void        *m_pObject;

};

typedef CMLLNode*   MPOS;



template<class T>
class CMultiLinkList
{
    protected:
        
            
    
    public:

                    CMultiLinkList()
                    {
                        m_nElements = 0;
                        m_pHead = NULL;
                    }

                    ~CMultiLinkList()
                    {
                        RemoveAll();
                    }

                    // Replacement for GetHeadPosition().
                    operator MPOS()     { return m_pHead; }
        
        uint32      GetCount() const        { return m_nElements; }
        uint32      GetSize() const     { return m_nElements; }
        int         IsEmpty()  const        { return !m_pHead; }

        T           GetHead()   const;
        T           GetTail()   const;

        T           RemoveHead();
        T           RemoveTail();

        MPOS        AddHead(T newHead, CMLLNode *pLinkInfo);
        MPOS        AddTail(T newTail, CMLLNode *pLinkInfo);

        void        RemoveAll();

        MPOS        GetHeadPosition()   const   { return m_pHead; }
        MPOS        GetTailPosition()   const   { return (m_pHead ? m_pHead->m_pGPrev : NULL); }

        T           GetNext(MPOS &pos)    const;
        T           GetPrev(MPOS &pos)    const;
        
        T           GetAt(MPOS pos);
        
        void        RemoveAt(MPOS pos);

        MPOS        InsertBefore(MPOS pos, T el, CMLLNode *pLinkInfo);
        MPOS        InsertAfter(MPOS pos, T el, CMLLNode *pLinkInfo);

        MPOS        Find(T searchFor, uint32 *pIndex=NULL)        const;
        uint32      FindElement(T searchFor, MPOS *pPos=NULL) const;
        MPOS        FindIndex(uint32 index)                   const;




    protected:

        uint32          m_nElements;
        CMLLNode        *m_pHead;

};



template<class T>
T CMultiLinkList<T>::GetHead() const
{
    ASSERT(m_pHead);
    return (T)m_pHead->m_pObject;
}


template<class T>
T CMultiLinkList<T>::GetTail() const
{
    ASSERT(m_pHead);
    return (T)m_pHead->m_pGPrev->m_pObject;
}


template<class T>
T CMultiLinkList<T>::GetNext(MPOS &pos) const
{
    T           ret = (T)pos->m_pObject;
    
    if (pos->m_pGNext == m_pHead)
        pos = NULL;
    else
        pos = pos->m_pGNext;

    return ret;
}


template<class T>
T CMultiLinkList<T>::GetPrev(MPOS &pos) const
{
    T           ret = (T)pos->m_pObject;

    if (pos->m_pGPrev == m_pHead)
        pos = NULL;
    else
        pos = pos->m_pGPrev;

    return ret;
}


template<class T>
T CMultiLinkList<T>::RemoveHead()
{
    T   ret;

    ASSERT(m_pHead);
    ret = (T)m_pHead->m_pObject;
    RemoveAt(m_pHead);
    
    return ret;
}


template<class T>
T CMultiLinkList<T>::RemoveTail()
{
    T   ret;

    ASSERT(m_pHead);
    ret = (T)m_pHead->m_pGPrev->m_pObject;
    RemoveAt(m_pHead->m_pGPrev);
    
    return ret;
}


template<class T>
MPOS CMultiLinkList<T>::AddHead(T newHead, CMLLNode *pLinkInfo)
{
    if (m_pHead)
    {
        return InsertBefore(m_pHead, newHead, pLinkInfo);
    }
    else
    {
        pLinkInfo->m_pGNext = pLinkInfo;
        pLinkInfo->m_pGPrev = pLinkInfo;
        pLinkInfo->m_pObject = newHead;

        m_pHead = pLinkInfo;
        ++m_nElements;

        return m_pHead;
    }
}


template<class T>
MPOS CMultiLinkList<T>::AddTail(T newTail, CMLLNode *pLinkInfo)
{
    if (m_pHead)
        return InsertAfter(m_pHead->m_pGPrev, newTail, pLinkInfo);
    else
        return AddHead(newTail, pLinkInfo);
}


template<class T>
void CMultiLinkList<T>::RemoveAll()
{
    m_pHead = NULL;
    m_nElements = 0;
}


template<class T>
T CMultiLinkList<T>::GetAt(MPOS pos)
{
    return (T)pos->m_pObject;
}


template<class T>
void CMultiLinkList<T>::RemoveAt(MPOS pos)
{
    ASSERT(m_nElements > 0);
    
    if (pos == m_pHead)
        m_pHead = m_pHead->m_pGNext;
    
    pos->m_pGPrev->m_pGNext = pos->m_pGNext;
    pos->m_pGNext->m_pGPrev = pos->m_pGPrev;

	pos->m_pGNext = pos->m_pGPrev = pos;
    
    --m_nElements;
    if (m_nElements == 0)
        m_pHead = NULL;
}


template<class T>
MPOS CMultiLinkList<T>::InsertBefore(MPOS pos, T el, CMLLNode *pLinkInfo)
{
    pos->m_pGPrev->m_pGNext = pLinkInfo;
    pLinkInfo->m_pGPrev = pos->m_pGPrev;
    pLinkInfo->m_pGNext = pos;
    pos->m_pGPrev = pLinkInfo;

    pLinkInfo->m_pObject = el;
    
    ++m_nElements;
    if (pLinkInfo->m_pGNext == m_pHead)
        m_pHead = pLinkInfo;

    return pLinkInfo;
}


template<class T>
MPOS CMultiLinkList<T>::InsertAfter(MPOS pos, T el, CMLLNode *pLinkInfo)
{
    pos->m_pGNext->m_pGPrev = pLinkInfo;
    pLinkInfo->m_pGNext = pos->m_pGNext;
    pLinkInfo->m_pGPrev = pos;
    pos->m_pGNext = pLinkInfo;

    pLinkInfo->m_pObject = el;
    
    ++m_nElements;
    return pLinkInfo;
}


template<class T>
MPOS CMultiLinkList<T>::Find(T searchFor, uint32 *pIndex) const
{
    CMLLNode *pCur;
    uint32      index=0;
    
    if (m_pHead)
    {
        pCur = m_pHead;
        do
        {
            
            if ((T)pCur->m_pObject == searchFor)
            {
                if (pIndex)
                    *pIndex = index;

                return pCur;
            }

            pCur = pCur->m_pGNext;
            ++index;

        } while (pCur != m_pHead);
    }

    if (pIndex)
        *pIndex = BAD_INDEX;

    return NULL;
}


template<class T>
uint32 CMultiLinkList<T>::FindElement(T searchFor, MPOS *pPos) const
{
    MPOS    pos;
    uint32  index;
    
    pos = Find(searchFor, &index);

    if (pPos)
        *pPos = pos;

    return index;
}


template<class T>
MPOS CMultiLinkList<T>::FindIndex(uint32 index) const
{
    CMLLNode    *pCur;
    
    if (index >= m_nElements)
        return NULL;

    pCur = m_pHead;
    do
    {
        
        if (index == 0)
            return pCur;

        pCur = pCur->m_pGNext;
        --index;

    } while (pCur != m_pHead);

    ASSERT(FALSE);    // Shouldn't ever get here.
    return NULL;
}




// Useful template function if your linked list is made up of pointers.
template<class T>
void MDeleteAndRemoveElements(CMultiLinkList<T> &theList)
{
    MPOS        pos;

    for (pos=theList.GetHeadPosition(); pos != NULL;)
        delete theList.GetNext(pos);

    theList.RemoveAll();
}


#endif // __MULTILINKLIST_H__






