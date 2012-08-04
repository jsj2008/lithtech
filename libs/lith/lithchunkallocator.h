/****************************************************************************
;
;   MODULE:     LithChunkAllocator (.H)
;
;   PURPOSE:    Class to allocate items in chunks for efficency.
;
;   HISTORY:    04/14/99 [blb]
;
;   NOTICE:     Copyright (c) 1999 MONOLITH, Inc.
;
****************************************************************************/

#ifndef __LITHCHUNKALLOCATOR_H__
#define __LITHCHUNKALLOCATOR_H__


#ifndef __LITH_H__
#include "lith.h"
#endif

#ifndef __LITHBASELIST_H__
#include "lithbaselist.h"
#endif


// User can derive new list classes from this class, or use it as is.
template<class ItemType>
class CLithChunkAllocator
{
public:
    // Constructors and destructors
    CLithChunkAllocator() { m_bInitialized = FALSE; };
    ~CLithChunkAllocator() { Term(); };

    // Initialization and Termination
    inline BOOL Init(unsigned int nChunkSize, unsigned int nInitialGrowSize = 0);
    inline void Term();

    // Object allocation and de-allocation
    inline ItemType* Alloc();
    inline void Free(ItemType* pItem);

    // Used to clean up all allocated items and free all chunks
    inline void FreeAllChunks();

    // Expand the free list by this given number of chunks (number of items = Chunk Size * NumChunks)
    inline BOOL GrowChunks(unsigned int nNumChunks);

    // Change the chunk allocation size
    inline void SetChunkSize(unsigned int nChunkSize) { m_nChunkSize = nChunkSize; };

private:

    // class for a chunk item
    class CChunk : public CLithBaseListItem<CChunk>
    {
    public:
        ItemType* m_pItemAry;
    };

    // allocate a new chunk
    BOOL AllocChunk();

    // get item from the free list
    ItemType* GetFromFreeList() 
    {
        ItemType* pItem = m_pFreeList;
        if (m_pFreeList != NULL) m_pFreeList = *((ItemType**)m_pFreeList);
        return pItem;
    };

    // add item to the free list
    void AddToFreeList(ItemType* pItem)
    {
        *((ItemType**)(pItem)) = m_pFreeList;
        m_pFreeList = pItem;
    };

    // internal member functions
    BOOL m_bInitialized;                    // if TRUE object is initialized
    unsigned int m_nChunkSize;              // size of chunks
    CLithBaseList<CChunk> m_lstChunks;      // list of all chunks
    ItemType* m_pFreeList;                  // item free list
};


template<class ItemType>
BOOL CLithChunkAllocator<ItemType>::Init(unsigned int nChunkSize, unsigned int nInitialChunkGrowSize)
{
    BOOL bRetVal = TRUE;

    // we must be able to fit our next pointer inside each item
    if (sizeof(ItemType) < sizeof(ItemType*)) return FALSE;  

    // set the initial chunk size
    m_nChunkSize = nChunkSize;

    // grow initial items on free list
    if (nInitialChunkGrowSize != 0) 
    {
        bRetVal = GrowChunks(nInitialChunkGrowSize);
    }

    // if we have succeeded then calss is initialized
    if (bRetVal == TRUE) m_bInitialized = TRUE;

    return bRetVal;
};


template<class ItemType>
void CLithChunkAllocator<ItemType>::Term()
{
    // if class is not initialized then just exit
    if (!m_bInitialized) return;

    // free all chunks in memory
    FreeAllChunks();

    // class is no longer initialized
    m_bInitialized = FALSE;
};


template<class ItemType>
ItemType* CLithChunkAllocator<ItemType>::Alloc()
{
    // are there any free items on the free list
    if (m_pFreeList != NULL) return GetFromFreeList();

    // if there are no items on the free list we must allocate another chunk
    if (!AllocChunk()) return NULL;

    // are there any free items on the free list
    return GetFromFreeList();
};


template<class ItemType>
void CLithChunkAllocator<ItemType>::Free(ItemType* pItem)
{
    // if item is null just exit
    if (pItem == NULL) return;

    // add the item to the free list
    AddToFreeList(pItem);
};


template<class ItemType>
void CLithChunkAllocator<ItemType>::FreeAllChunks()
{
    CChunk* pChunk;

    // check if the class is currently initialized
    if (!m_bInitialized) return;

    // free all the chunks
    pChunk = m_lstChunks.GetFirst();
    while (pChunk != NULL)
    {
        // delete the chunk
        m_lstChunks.Delete(pChunk);
        delete [] pChunk->m_pItemAry;
        delete pChunk;

        // get the next chunk
        pChunk = m_lstChunks.GetFirst();
    }

    // clear the freelist
    m_pFreeList = NULL;
};


template<class ItemType>
BOOL CLithChunkAllocator<ItemType>::GrowChunks(unsigned int nNumChunks)
{
    while (nNumChunks > 0)
    {
        if (!AllocChunk()) return FALSE;
        nNumChunks--;
    }
    return TRUE;
};


template<class ItemType>
BOOL CLithChunkAllocator<ItemType>::AllocChunk()
{
    CChunk* pNewChunk;
    unsigned int i;
    ItemType* pItem;

    // allocate the new chunk
    pNewChunk = new CChunk;
    if (pNewChunk == NULL) return FALSE;

    // allocate array of items for new chunk
    pNewChunk->m_pItemAry = new ItemType[m_nChunkSize];
    if (pNewChunk->m_pItemAry == NULL) return FALSE;

    // add new items to the free list (we store next pointer inside the item!)
    pItem = pNewChunk->m_pItemAry;  
    for (i = 0; i < m_nChunkSize; i++)
    {
        // add to head of the free list
        AddToFreeList(pItem);

        // go to next item
        pItem++;
    }

    // add new chunk item to the chunk list
    m_lstChunks.Insert(pNewChunk);

    // if we made it here everything is OK
    return TRUE;
};


#endif 
