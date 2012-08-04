
#ifndef __GENHASH_H__
#define __GENHASH_H__

#ifndef __GENLIST_H__
#include "genlist.h"
#endif

#ifndef __DYNARRAY_H__
#include "dynarray.h"
#endif

#ifndef __LINKLIST_H__
#include "linklist.h"
#endif

#ifndef __GOODLINKLIST_H__
#include "goodlinklist.h"
#endif


// When using a GenHash, you must provide one of these.
template<class T>
class GenHashHelper
{
public:

    // This is called to generate a hash code for your data.
    // Don't mod it by anything - GenHash will do it for you.
    virtual uint32  GetHashCode(const T &pObject)=0;

    // Compare two objects.
    virtual LTBOOL  Compare(const T &pObject1, const T &pObject2)=0;
};


// Note: you can NOT use a hash table as a bucket type (because there isn't space in
// the GenListPos iterators).
template<class T, class BucketType>
class GenHash : public GenList<T>
{
public:

                    GenHash();
                    ~GenHash();

    // Sets up list size and hash generator.
    LTBOOL          Init(
        uint32 nLists,
        GenHashHelper<T> *pHelper,
        uint32 listCacheSize=0);

    void            Term();


// GenList implementation.
public:
    
    virtual GenListPos  GenBegin() const;
    virtual LTBOOL      GenIsValid(const GenListPos &pos) const;
    virtual T           GenGetNext(GenListPos &pos) const;
    virtual T           GenGetAt(GenListPos &pos) const;
    virtual LTBOOL      GenAppend(T &toAppend);
    virtual void        GenRemoveAt(GenListPos pos);
    virtual void        GenRemoveAll();
    virtual uint32      GenGetSize() const;
    virtual LTBOOL      GenCopyList(const GenList<T> &other);
    virtual LTBOOL      GenAppendList(const GenList<T> &other);
    virtual LTBOOL      GenFindElement(const T &toFind, GenListPos &thePos) const;


private:
    
    // The hash table.
    BucketType          *m_pLists;
    uint32              m_nLists;

    // Tracked as elements are added and removed.
    uint32              m_nElements;

    // Does the actual hash code calculations.
    GenHashHelper<T>    *m_pHelper;
};


template<class T, class BucketType>
inline GenHash<T, BucketType>::GenHash()
{
    m_pLists = 0;
    m_nLists = 0;
    m_nElements = 0;
    m_pHelper = NULL;
}


template<class T, class BucketType>
inline GenHash<T, BucketType>::~GenHash()
{
    Term();
}


template<class T, class BucketType>
inline LTBOOL GenHash<T, BucketType>::Init(
    uint32 nLists,
    GenHashHelper<T> *pHelper,
    uint32 listCacheSize)
{
    uint32 i;

    // Just does this so you'll get a compile error if you try to use a hash table
    // as a bucket (which you can't do!)
    BucketType::CheckSupportHashBucket();

    Term();

    m_pLists = new BucketType[nLists];
    if (!m_pLists)
        return FALSE;
    m_nLists = nLists;

    for (i=0; i < m_nLists; i++)
        m_pLists[i].GenSetCacheSize(listCacheSize);

    m_pHelper = pHelper;
    return TRUE;
}


template<class T, class BucketType>
inline void GenHash<T, BucketType>::Term()
{
    if (m_pLists)
    {
        delete [] m_pLists;
        m_pLists = NULL;
    }

    m_nLists = 0;
    m_nElements = 0;
    m_pHelper = NULL;
}


template<class T, class BucketType>
inline GenListPos GenHash<T, BucketType>::GenBegin() const
{
    uint32 i;

    if (m_nElements == 0)
    {
        // Return an invalid one.
        return GenListPos(m_nLists, 0);
    }
    else
    {
        // Find the first element.
        for (i=0; i < m_nLists; i++)
        {
            if (m_pLists[i].GenGetSize() > 0)
            {
                return GenListPos(i, m_pLists[i].GenBegin());
            }
        }

        // Ok, something's wrong with m_nElements.
        ASSERT(FALSE);
        return GenListPos(m_nLists, 0);
    }
}


template<class T, class BucketType>
inline LTBOOL GenHash<T, BucketType>::GenIsValid(const GenListPos &pos) const
{
    return pos.m_Index < m_nLists;
}


template<class T, class BucketType>
inline T GenHash<T, BucketType>::GenGetNext(GenListPos &pos) const
{
    T ret;
    BucketType *pList;

    
    // Get the item.
    ASSERT(pos.m_Index < m_nLists);

    pList = &m_pLists[pos.m_Index];
    ret = pList->GenGetNext(pos.GetSubIndex());
    
    // Increment the index (and move to a used list if necessary).
    if (!pList->GenIsValid(pos.GetSubIndex()))
    {
        ++pos.m_Index;
        for (; pos.m_Index < m_nLists; pos.m_Index++)
        {
            if (m_pLists[pos.m_Index].GetSize() > 0)
            {
                pos.m_SubIndex = m_pLists[pos.m_Index].GenBegin();
                break;
            }
        }
    }

    return ret;
}


template<class T, class BucketType>
inline T GenHash<T, BucketType>::GenGetAt(GenListPos &pos) const
{
    // Get the item.
    ASSERT(pos.m_Index < m_nLists);
    return m_pLists[pos.m_Index].GenGetAt(pos.GetSubIndex());
}


template<class T, class BucketType>
inline LTBOOL GenHash<T, BucketType>::GenAppend(T &toAppend)
{
    ASSERT(m_pLists && m_pHelper);
    if (m_pLists[m_pHelper->GetHashCode(toAppend) % m_nLists].GenAppend(toAppend))
    {
        ++m_nElements;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


template<class T, class BucketType>
inline void GenHash<T, BucketType>::GenRemoveAt(GenListPos pos)
{
    ASSERT(pos.m_Index < m_nLists);
    m_pLists[pos.m_Index].GenRemoveAt(pos.GetSubIndex());
    m_nElements--;
}


template<class T, class BucketType>
inline void GenHash<T, BucketType>::GenRemoveAll()
{
    uint32 i;

    for (i=0; i < m_nLists; i++)
    {
        m_pLists[i].GenRemoveAll();
    }

    m_nElements = 0;
}


template<class T, class BucketType>
inline uint32 GenHash<T, BucketType>::GenGetSize() const
{
    return m_nElements;
}


template<class T, class BucketType>
inline LTBOOL GenHash<T, BucketType>::GenCopyList(const GenList<T> &other)
{
    Term();
    return GenAppendList(other);
}


template<class T, class BucketType>
inline LTBOOL GenHash<T, BucketType>::GenAppendList(const GenList<T> &other)
{
    GenListPos pos;

    for (pos=other.GenBegin(); other.GenIsValid(pos);)
    {
        if (!GenAppend(other.GenGetNext(pos)))
        {
            Term();
            return FALSE;
        }
    }

    return TRUE;
}


template<class T, class BucketType>
inline LTBOOL GenHash<T, BucketType>::GenFindElement(const T &toFind, GenListPos &thePos) const
{
    BucketType *pList;
    GenListPos pos;

    ASSERT(m_pLists && m_pHelper);

    // Trivial reject.
    if (m_nElements == 0)
        return FALSE;

    thePos.m_Index = m_pHelper->GetHashCode(toFind) % m_nLists;
    pList = &m_pLists[thePos.m_Index];
    for (pos=pList->GenBegin(); pList->GenIsValid(pos);)
    {
        thePos.m_SubIndex = pos;
        if (m_pHelper->Compare(pList->GenGetNext(pos), toFind))
        {
            return TRUE;
        }
    }

    return FALSE;
}


#endif

