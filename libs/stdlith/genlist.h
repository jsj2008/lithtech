
/*

GenList.h

This module defines the GenList base class that can be used to abstract out
differences between list types. Very useful in the preprocessor so it doesn't
have to manually make copies of different list types.

If you have a GenList object, you can iterate it like this:

GenListPos pos;
for(pos=pList->GenBegin(); pList->GenIsValid(pos);)
    pObj = pList->GenGetNext(pos);

*/

#ifndef __GENLIST_H__
#define __GENLIST_H__

#ifndef __STDLITHDEFS_H__
#include "stdlithdefs.h"
#endif

class GenListPos;


// Arrays and lists only use this.
class BaseGenListPos
{
public:

    void        operator=(const GenListPos &other);

    // In debug, the elements are split out so linked lists can maintain a loop
    // index as you iterate.
    #ifdef _DEBUG
        void    *m_Pos;
        uint32  m_Index;
    #else
        union
        {
            void    *m_Pos;
            uint32  m_Index;
        };
    #endif
};


class GenListPos : public BaseGenListPos
{
public:
                GenListPos()                            {}
                GenListPos(void *pos)                   {m_Pos = pos;}
                GenListPos(uint32 index)                    {m_Index = index;}
                GenListPos(uint32 index, BaseGenListPos subIndex)   {m_Index = index; m_SubIndex = subIndex;}
                GenListPos(uint32 index, uint32 subIndex)   {m_Index = index; m_SubIndex.m_Index = subIndex;}
                
                #ifdef _DEBUG
                    GenListPos(void *pos, uint32 index) {m_Pos = pos; m_Index = index;}
                #endif

    // Use this to get m_SubIndex as a GenListPos.
    inline GenListPos&  GetSubIndex()
    {
        return *((GenListPos*)&m_SubIndex);
    }


    // For GenHash.
    BaseGenListPos  m_SubIndex;
};


template<class T>
class GenList
{
public:

    // Get the start for iterating.
    virtual GenListPos  GenBegin() const = 0;
    
    // Returns TRUE if you're done iterating.
    virtual LTBOOL      GenIsValid(const GenListPos &pos) const = 0;
    
    // Get the next element and increment the iterator.     
    virtual T           GenGetNext(GenListPos &pos) const = 0;
    
    // Get the element at the specified position without incrementing or decrementing.
    virtual T           GenGetAt(GenListPos &pos) const = 0;
    
    // Append an element. Be careful with your usage of GenGetNext and GenGetAt because
    // if you are only using GenGetNext and you append, you might not iterate over the
    // element you've appended. To append while iterating, use a loop like this:
    // GenListPos pos;
    // for (pos=pList->GenBegin(); pList->GenIsValid(pos);)
    // {
    //     pCurItem = pList->GenGetAt(pos);
    //     pList->Append(pSomeItem);
    //     pList->GetNext(pos); // Increment here.
    // }
    // on 
    virtual LTBOOL      GenAppend(T &toAppend)=0;

    // NOTE: this trashes all iterators so you must restart iteration.
    // Note: this does a search through all elements for CLinkedLists.
    virtual void        GenRemoveAt(GenListPos pos)=0;

    // Remove all elements.
    virtual void        GenRemoveAll()=0;

    // Returns the number of elements in the list.
    virtual uint32      GenGetSize() const = 0;

    // Copy another list.
    virtual LTBOOL      GenCopyList(const GenList<T> &other)=0;

    // Append another list.
    virtual LTBOOL      GenAppendList(const GenList<T> &other)=0;

    // Search through the list for an item. If it finds it, fills in 
    // the position it was found at.
    // NOTE: this does comparisons with memcmp. This so objects that go in lists
    // don't have to define operator==.
    virtual LTBOOL      GenFindElement(const T &toFind, GenListPos &thePos) const = 0;

    // Set cache size. This is only meaningful to CMoArrays.
    virtual void        GenSetCacheSize(uint32 size) {}
};



// Inlines.
inline void BaseGenListPos::operator=(const GenListPos &other)
{
    m_Index = other.m_Index;

    #ifdef _DEBUG
        m_Pos = other.m_Pos;
    #endif
}


#endif






