// ------------------------------------------------------------------ //
//
//  FILE      : DStreamOpenQueueMgr.h
//
//  PURPOSE   : This is a mgr for keeping a most recently used queue 
//              of ILTStream's open.
//
//  CREATED   : 1999
//
//  COPYRIGHT : Monolith Productions, Inc 1999, 2000
//              All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __DSTREAMOPENQUEUEMGR_H__
#define __DSTREAMOPENQUEUEMGR_H__


#ifndef __LITH_H__
#include "lith.h"
#endif

#ifndef __CLIENTMGR_H__
#include "clientmgr.h"
#endif

#ifndef __CLIENTDE_IMPL_H__
#include "clientde_impl.h"
#endif


class CDStreamOpenQueueMgr;


////////////////////////////////////////////////////////////////////////////
// class CDStreamOpenQueueItem
////////////////////////////////////////////////////////////////////////////
class CDStreamOpenQueueItem : public CBaseListItem
{
public:
    CDStreamOpenQueueItem(CDStreamOpenQueueMgr* pDStreamOpenQueueMgr) 
    { 
        m_pDStream = LTNULL; 
        m_nLockCount = 0; 
        m_DStreamFileRef_.m_pFilename = LTNULL;
        m_pDStreamOpenQueueMgr = pDStreamOpenQueueMgr; 
        m_nSaveSeekPos = 0;
        file_name = NULL;
    };

    ~CDStreamOpenQueueItem() { 
        if (m_pDStream != LTNULL) Close();
        if (file_name != LTNULL) {
            delete file_name;
        }
    };

    // open a ILTStream (calls close if old one was open)
    LTRESULT Open(const char* sFileName = LTNULL);

    // close a ILTStream and set to LTNULL
    LTRESULT Close();

    // accessor for the ILTStream this locks it open
    ILTStream* LockDStream();

    // unlock the ILTStream for this item
    void UnLockDStream();

    // return the file name currently assigned to this item
    const char* GetFileName() { return m_DStreamFileRef_.m_pFilename; };

    // list operators from base class to get next and previous item in list
    CDStreamOpenQueueItem* Next() { return (CDStreamOpenQueueItem*)CBaseListItem::Next(); };
    CDStreamOpenQueueItem* Prev() { return (CDStreamOpenQueueItem*)CBaseListItem::Prev(); };

private:
    // mgr class needs access to this class
    friend class CDStreamOpenQueueMgr;

    // open the dstream
    LTRESULT OpenDStream();

    // close the dstream
    LTRESULT CloseDStream();

    // pointer to the ILTStream that is associated with this queue item
    // if this is LTNULL then this item exists in the closed list otherwise it is in the opened list
    ILTStream* m_pDStream;

    // pointer to the mgr that this item belongs in
    CDStreamOpenQueueMgr* m_pDStreamOpenQueueMgr;

    // lock count for this item if this is greater than zero then the item is locked
    unsigned int m_nLockCount;

    // name and other information needed to open the file
    FileRef m_DStreamFileRef_;

    //the allocated space for our failename.  the FileRef points to it, and we deallocated it.
    char *file_name;

    // save seek pos when file is closed
    uint32 m_nSaveSeekPos;
};


////////////////////////////////////////////////////////////////////////////
// class CDStreamOpenQueueList
////////////////////////////////////////////////////////////////////////////
class CDStreamOpenQueueList : public CLTBaseList
{
public:
    // version of all the base functions that set our number of items counter
    CDStreamOpenQueueList() { m_nNumItems = 0; CLTBaseList::CLTBaseList(); };
    void Insert(CBaseListItem* pItem) { m_nNumItems++; CLTBaseList::Insert(pItem); };         
    void InsertFirst(CBaseListItem* pItem) { m_nNumItems++; CLTBaseList::InsertFirst(pItem); };                               
    void InsertLast(CBaseListItem* pItem) { m_nNumItems++; CLTBaseList::InsertLast(pItem); };
    void InsertAfter(CBaseListItem* pBeforeItem, CBaseListItem* pNewItem) { m_nNumItems++; CLTBaseList::InsertAfter(pBeforeItem, pNewItem); };
    void InsertBefore(CBaseListItem* pAfterItem, CBaseListItem* pNewItem) { m_nNumItems++; CLTBaseList::InsertBefore(pAfterItem, pNewItem); };
    void Delete(CBaseListItem* pItem) { m_nNumItems--; CLTBaseList::Delete(pItem); };                                 
    void FastDeleteAll() { m_nNumItems = 0; CLTBaseList::FastDeleteAll(); };

    // get the number of items in the list
    unsigned int GetNumItems() { return m_nNumItems; };

    // get the first item in the list
    CDStreamOpenQueueItem* GetFirst() { return (CDStreamOpenQueueItem*)CLTBaseList::GetFirst(); };

    // get the last item in the list
    CDStreamOpenQueueItem* GetLast() { return (CDStreamOpenQueueItem*)CLTBaseList::GetLast(); };

private:
    // number of items in the list
    unsigned int m_nNumItems;
};


////////////////////////////////////////////////////////////////////////////
// class CDStreamOpenQueueMgr
////////////////////////////////////////////////////////////////////////////
class CDStreamOpenQueueMgr 
{
public:

    CDStreamOpenQueueMgr() { m_bInitialized = LTFALSE; };
    CDStreamOpenQueueMgr(int nNumItems) { m_bInitialized = LTFALSE; Init(nNumItems); };
    ~CDStreamOpenQueueMgr() { if (m_bInitialized) Term(); };

    // Initialize the queue with the specified number of items
    void Init(int nNumItems);

    // Terminate the queue releasing all items
    void Term();

    // Create a new item
    CDStreamOpenQueueItem* Create(const char* sFileName = LTNULL);

    // Destroy an item that was created
    void Destroy(CDStreamOpenQueueItem* pItem);

    // Destroy all items
    void DestroyAll();

    // close all currently opened items
    LTRESULT CloseAll();

private:
    // item class needs access to this class
    friend class CDStreamOpenQueueItem;

    // reduce the number of opened files down to the max if possible
    void ReduceOpenedItems();

    // helper functions to critical sections
    void EnterCriticalSection() { ::EnterCriticalSection(&m_CriticalSection); };
    void LeaveCriticalSection() { ::LeaveCriticalSection(&m_CriticalSection); };

    // critical section to control access to this class from multiple threads
    CRITICAL_SECTION m_CriticalSection;

    // list of currently opened items
    CDStreamOpenQueueList m_lstOpenedItems;

    // list of currently closed items
    CDStreamOpenQueueList m_lstClosedItems;

    // maximum number of open items we are targeting to be opened at once (this can be exceeded if more are locked all at the same time)
    unsigned int m_nMaxOpenedItems;

    // if true mgr was initialized
    LTBOOL m_bInitialized;
};

#endif

