
#ifndef __STRUCT_BANK_H__
#define __STRUCT_BANK_H__

#ifndef __L_ALLOCATOR_H__
#include "l_allocator.h"
#endif


// This can be set to TRUE for debug builds, which will cause the structbanks
// to just allocate and free memory like normal.  Helps find memory leaks.
extern int32 g_bDebugStructBanks;

typedef struct StructLink_t {
    struct StructLink_t *m_pSLNext;
} StructLink;

typedef struct StructBankPage_t {
    struct StructBankPage_t *m_pNext;
    unsigned long   m_nObjects; // How many objects are in this page?
    uint32          m_Data[1];
} StructBankPage;

typedef struct StructBank_t {
    // Struct size.
    unsigned long m_StructSize;
    
    // The actual size we allocate with (aligned to 4 bytes and with 4 bytes
    // extra for the StructLink).
    unsigned long m_AlignedStructSize;
    
    // How many structs per page.
    unsigned long m_CacheSize;

    // How many pages have we allocated?
    unsigned long m_nPages;
    
    // Total number of objects this StructBank has allocated.
    unsigned long m_nTotalObjects;

    // The first page.
    StructBankPage *m_PageHead;

    // The free list.
    StructLink *m_FreeListHead;
} StructBank;


// You can use this to declare StructBanks.. it'll make it safe to
// call sb_Term even if you haven't called sb_Init yet.
#define DECLARE_STRUCTBANK(name) \
    StructBank name = {0, 0, 0, 0, 0};

// Internal.. don't call this.
	int32 sb_AllocateNewStructPage(StructBank *pBank, uint32 nAllocations);

// Initialize.. pass in the size of your structure and how much to cache.
void sb_Init(StructBank *pBank, uint32 structSize, uint32 cacheSize);
	
// Initialize and preallocate some (to avoid allocation the first time).
// Returns 1 if successful, 0 otherwise.  Even if it returns 0, the StructBank
// will be initialized and you might be able to allocate thru it.
int32 sb_Init2(StructBank *pBank, uint32 structSize, uint32 cacheSize,
		uint32 nPreallocations);

// This puts everything that has been allocated into the free list.  It doesn't 
// actually free any memory.  It can be useful if you want easy access to a bunch
// of StructBank'd structures and don't want to worry about freeing them all individually.
void sb_FreeAll(StructBank *pBank);

	
// Shutdown.
void sb_Term(StructBank *pBank);
void sb_Term2(StructBank *pBank, int32 bCheckObjects);

// Returns TRUE if the object is currently allocated.
// Only returns FALSE in debug mode..
LTBOOL sb_IsObjectAllocated(StructBank *pBank, void *pObj);

// Allocate an instance.
inline void* sb_Allocate(StructBank *pBank) {
    StructLink *pRet;

    #ifdef _DEBUG
    if (g_bDebugStructBanks) {
        return g_DefAlloc.Alloc(pBank->m_StructSize);
    }
    #endif

    pRet = pBank->m_FreeListHead;
    if (!pRet) {
        sb_AllocateNewStructPage(pBank, pBank->m_CacheSize);
        pRet = pBank->m_FreeListHead;
        if (!pRet) {
            return 0;
        }

        ASSERT(pRet);
    }

    pBank->m_FreeListHead = pRet->m_pSLNext;
    return pRet;
}

// Allocate a zero-initted instance.    
inline void* sb_Allocate_z(StructBank *pBank) {
    StructLink *pRet;

    #ifdef _DEBUG
    if (g_bDebugStructBanks) {
        void *pVoidRet = g_DefAlloc.Alloc(pBank->m_StructSize);
        if (pVoidRet) {
            memset(pVoidRet, 0, pBank->m_StructSize);
        }
        return pVoidRet;
    }
    #endif

    pRet = pBank->m_FreeListHead;
    if (!pRet) {
        sb_AllocateNewStructPage(pBank, pBank->m_CacheSize);
        pRet = pBank->m_FreeListHead;
        if (!pRet) {
            return 0;
        }

        ASSERT(pRet);
    }

    pBank->m_FreeListHead = pRet->m_pSLNext;
    memset(pRet, 0, pBank->m_StructSize);
    return pRet;
}

inline void sb_Free(StructBank *pBank, void *pObj) {
    StructLink *pLink = (StructLink*)pObj;

    #ifdef _DEBUG
    memset(pObj, 0xEA, pBank->m_StructSize);

    if (g_bDebugStructBanks) {
        g_DefAlloc.Free(pObj);
        return;
    }
    #endif

    pLink->m_pSLNext = pBank->m_FreeListHead;
    pBank->m_FreeListHead = pLink;
}

#endif












