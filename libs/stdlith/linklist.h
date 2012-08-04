#ifndef __LINKLIST_H__
#define __LINKLIST_H__

#ifndef __GENLIST_H__
#include "genlist.h"
#endif

#ifndef __OBJECT_BANK_H__
#include "object_bank.h"
#endif


#ifndef ASSERT
    #define ASSERT
#endif

typedef void*   LPOS;


// NOTE: m_pPrev and m_pNext in this structure MUST match 
// CGenLLNode for LinkList_FindElementMemcmp.
template<class T>
class CLLNode
{
    public:
        
        CLLNode<T>      *m_pPrev;
        CLLNode<T>      *m_pNext;
        T               m_pData;
};

// NOTE: m_pPrev and m_pNext in this structure MUST match 
// CLLNode for LinkList_FindElementMemcmp.
class CGenLLNode
{
public:
    CGenLLNode      *m_pPrev;
    CGenLLNode      *m_pNext;
    char            m_Data[1];  // (Used to look at m_pData);
};


// This is defined as a separate function because it's called by the virtual
// GenFindElement and we don't want to generate a bunch of template code for
// every type of object we use in a list.
LPOS LinkList_FindElementMemcmp(
    const void *pHead,
    const void *pToFind,
    uint32 elementSize);


template<class T>
class CLinkedList : public GenList<T>
{
public:

                CLinkedList()
                {
                    m_nElements = 0;
                    m_pHead = NULL;
                }

                ~CLinkedList()
                {
                    RemoveAll();
                }

    
    uint32      GetSize() const     { return m_nElements; }
    uint32      GetCount() const        { return m_nElements; }
    int         IsEmpty()  const        { return !m_pHead; }

                operator LPOS() {return GetHeadPosition();}
                operator unsigned long() {return m_nElements;}

    T&          GetHead()               { ASSERT(m_pHead);  return m_pHead->m_pData; }
    T           GetHead()   const       { ASSERT(m_pHead);  return m_pHead->m_pData; }
    T&          GetTail()               { ASSERT(m_pHead);  return m_pHead->m_pPrev->m_pData; }
    T           GetTail()   const       { ASSERT(m_pHead);  return m_pHead->m_pPrev->m_pData; }

    T           RemoveHead();
    T           RemoveTail();

    LPOS        Append(T toAppend) {return AddTail(toAppend);}

    LPOS        AddHead(T newHead);
    LPOS        AddTail(T newTail);

 
    void        RemoveAll();


    LPOS            GetHeadPosition()   const   { return m_pHead; }
    LPOS            GetTailPosition()   const   { return (m_pHead ? m_pHead->m_pPrev : NULL); }

    inline T        GetNext(LPOS &pos) const
    {
        CLLNode<T> *pLink = ((CLLNode<T>*)pos);
        T ret = pLink->m_pData;
        
        if (pLink->m_pNext == m_pHead)
            pos = NULL;
        else
            pos = pLink->m_pNext;

        return ret;
    }


    inline T        GetPrev(LPOS &pos) const
    {
        CLLNode<T> *pLink = ((CLLNode<T>*)pos);
        T ret = pLink->m_pData;

        if (pLink == m_pHead)
            pos = NULL;
        else
            pos = pLink->m_pPrev;

        return ret;
    }

    T&          GetAt(LPOS &lPos) const       { return ((CLLNode<T>*)lPos)->m_pData; }
    
    void        SetAt(LPOS &lPos, T el)       { ((CLLNode<T>*)lPos)->m_pData = el; }
    void        RemoveAt(LPOS lPos);
    LTBOOL      RemoveElement(T el);

    LPOS            InsertBefore(LPOS lPos, T el);
    LPOS            InsertAfter(LPOS lPos, T el);

    // Find and compare with T::operator==.
    LPOS            Find(T searchFor)         const;
    
    LPOS            FindIndex(uint32 index)   const;
    
    // Yes, we can be a hash bucket.
    static void     CheckSupportHashBucket() {}


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

        return GetNext(*((LPOS*)&pos.m_Pos));
    }

    virtual T           GenGetAt(GenListPos &pos) const
    {
        return GetAt(*((LPOS*)&pos.m_Pos));
    }

    virtual LTBOOL      GenAppend(T &toAppend)
    {
        Append(toAppend);
        return TRUE;
    }

    virtual void        GenRemoveAt(GenListPos pos)
    {
        RemoveAt((LPOS)pos.m_Pos);
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
        thePos.m_Pos = LinkList_FindElementMemcmp(
            m_pHead,
            &toFind,
            sizeof(T));

        return !!thePos.m_Pos;
    }


protected:

    uint32      m_nElements;
    CLLNode<T>  *m_pHead;

    // ObjectBank for the links.
    typedef CLLNode<T>  MyTNode;
    ObjectBank<MyTNode> m_LinkBank;
};



template<class T>
T CLinkedList<T>::RemoveHead()
{
    T   ret;

    ASSERT(m_pHead);
    ret = m_pHead->m_pData;
    RemoveAt(m_pHead);
    
    return ret;
}


template<class T>
T CLinkedList<T>::RemoveTail()
{
    T   ret;

    ASSERT(m_pHead);
    ret = m_pHead->m_pPrev->m_pData;
    RemoveAt(m_pHead->m_pPrev);
    
    return ret;
}


template<class T>
LPOS CLinkedList<T>::AddHead(T newHead)
{
    CLLNode<T> *pNew;
    
    
    pNew = m_LinkBank.Allocate();
    if (!pNew)
        return NULL;

    pNew->m_pData = newHead;
    if (m_pHead)
    {
        pNew->m_pNext = m_pHead;
        pNew->m_pPrev = m_pHead->m_pPrev;
        pNew->m_pNext->m_pPrev = pNew;
        pNew->m_pPrev->m_pNext = pNew;
    }
    else
    {
        pNew->m_pNext = pNew;
        pNew->m_pPrev = pNew;
    }

    m_pHead = pNew;
    ++m_nElements;

    return m_pHead;
}


template<class T>
LPOS CLinkedList<T>::AddTail(T newTail)
{
    if (m_pHead)
        return InsertAfter((LPOS)m_pHead->m_pPrev, newTail);
    else
        return AddHead(newTail);
}


template<class T>
void CLinkedList<T>::RemoveAll()
{
    CLLNode<T>  *pCur, *pNext;

    if (m_pHead)
    {
        pCur = m_pHead;
        do
        {
            pNext = pCur->m_pNext;
            m_LinkBank.Free(pCur);
            pCur = pNext;
        } while (pCur != m_pHead);
            
        m_nElements = 0;
        m_pHead = NULL;
    }
}


template<class T>
void CLinkedList<T>::RemoveAt(LPOS pos)
{
    CLLNode<T>      *pNode = (CLLNode<T>*)pos;

    ASSERT(m_nElements > 0);
    
    pNode->m_pPrev->m_pNext = pNode->m_pNext;
    pNode->m_pNext->m_pPrev = pNode->m_pPrev;
    
    if (pos == m_pHead)
        m_pHead = m_pHead->m_pNext;

    m_LinkBank.Free(pNode);
    --m_nElements;

    if (m_nElements == 0)
        m_pHead = NULL;
}


template<class T>
LTBOOL CLinkedList<T>::RemoveElement(T el)
{
    LPOS pos;
    
    pos = Find(el);
    if (pos)
    {
        RemoveAt(pos);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


template<class T>
LPOS CLinkedList<T>::InsertBefore(LPOS lPos, T el)
{
    CLLNode<T> *pNode, *pNew;

    pNew = m_LinkBank.Allocate();
    if (!pNew)
        return NULL;

    pNode = (CLLNode<T>*)lPos;

    pNew->m_pData = el;
    pNew->m_pNext = pNode;
    pNew->m_pPrev = pNode->m_pPrev;
    pNew->m_pPrev->m_pNext = pNew->m_pNext->m_pPrev = pNew;
    
    ++m_nElements;
    if (pNew->m_pNext == m_pHead)
        m_pHead = pNew;

    return pNew;
}


template<class T>
LPOS CLinkedList<T>::InsertAfter(LPOS lPos, T el)
{
    CLLNode<T> *pNode, *pNew;

    pNew = m_LinkBank.Allocate();
    if (!pNew)
        return NULL;

    pNode = (CLLNode<T>*)lPos;

    pNew->m_pData = el;
    pNew->m_pNext = pNode->m_pNext;
    pNew->m_pPrev = pNode;
    pNew->m_pPrev->m_pNext = pNew->m_pNext->m_pPrev = pNew;
    
    ++m_nElements;
    return pNew;
}


template<class T>
LPOS CLinkedList<T>::Find(T searchFor) const
{
    CLLNode<T>  *pCur;
    
    if (m_pHead)
    {
        pCur = m_pHead;
        do
        {
        
            if (pCur->m_pData == searchFor)
                return pCur;

            pCur = pCur->m_pNext;
        
        } while (pCur != m_pHead);
    }

    return NULL;
}


template<class T>
LPOS CLinkedList<T>::FindIndex(uint32 index)  const
{
    CLLNode<T>  *pCur;
    
    if (index >= m_nElements)
        return NULL;

    pCur = m_pHead;
    do
    {
    
        if (--index == 0)
            return pCur;

        pCur = pCur->m_pNext;

    } while (pCur != m_pHead);

    ASSERT(FALSE);    // Shouldn't ever get here.
    return NULL;
}




// Useful template function if your linked list is made up of pointers.
template<class T>
void DeleteAndRemoveElements(CLinkedList<T> &theList)
{
    LPOS lPos;

    for (lPos=theList.GetHeadPosition(); lPos != NULL;)
        delete theList.GetNext(lPos);

    theList.RemoveAll();
}


#endif // __LINKLIST_H__






