//------------------------------------------------------------------
//
//  FILE      : DStreamOpenQueueMgr.cpp
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

#include "bdefs.h"
#include "dstreamopenqueuemgr.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);






// open a ILTStream (calls close if old one was open)
LTRESULT CDStreamOpenQueueItem::Open(const char* sFileName)
{
    m_pDStreamOpenQueueMgr->EnterCriticalSection();

    // make sure the lock count is 0
    if (m_nLockCount > 0) 
    {
        m_pDStreamOpenQueueMgr->LeaveCriticalSection();
        return LT_ERROR;
    }

    // close the file if it was open
    if (m_pDStream != LTNULL) Close();

    // check if we were passed in a new file name
    if (sFileName != LTNULL) 
    {
        // assign the new file name to the item
        if (file_name != LTNULL) {
            delete file_name;
        }

        LT_MEM_TRACK_ALLOC(m_DStreamFileRef_.m_pFilename = file_name = new char[strlen(sFileName)+1],LT_MEM_TYPE_FILE);
        if (m_DStreamFileRef_.m_pFilename == LTNULL)
        {
            m_pDStreamOpenQueueMgr->LeaveCriticalSection();
            return LT_ERROR;
        }
        strcpy(file_name, sFileName);
    }

    // open the dstream
    LTRESULT nResult = OpenDStream();

    m_pDStreamOpenQueueMgr->LeaveCriticalSection();

    return nResult;
};


// close a ILTStream and set to LTNULL
LTRESULT CDStreamOpenQueueItem::Close()
{
    m_pDStreamOpenQueueMgr->EnterCriticalSection();

    // make sure the lock count is 0
    if (m_nLockCount > 0) 
    {
        m_pDStreamOpenQueueMgr->LeaveCriticalSection();
        return LT_ERROR;
    }

    // check if it is already closed
    if (m_pDStream == LTNULL) 
    {
        m_pDStreamOpenQueueMgr->LeaveCriticalSection();
        return LT_OK;
    }

    // call the close function for the dstream
    LTRESULT nResult = CloseDStream();

    m_pDStreamOpenQueueMgr->LeaveCriticalSection();
    return nResult;
};


// accessor for the ILTStream this locks it open
ILTStream* CDStreamOpenQueueItem::LockDStream()
{
    m_pDStreamOpenQueueMgr->EnterCriticalSection();

    // increment lock count
    m_nLockCount++;

    // check if the ILTStream does not exist
    if (m_pDStream == LTNULL)
    {
        // open the ILTStream
        if (OpenDStream() != LT_OK)
        {
            // if it failed to open then we must error
            m_nLockCount--;
            m_pDStreamOpenQueueMgr->LeaveCriticalSection();
            return LTNULL;
        }
    }

    // if the stream is already opened then we just need to put it as the most recently used in the opened list
    else
    {
        // see if it is not already at the top before we move it
        if (m_pDStreamOpenQueueMgr->m_lstOpenedItems.GetFirst() != this)
        {
            m_pDStreamOpenQueueMgr->m_lstOpenedItems.Delete(this);
            m_pDStreamOpenQueueMgr->m_lstOpenedItems.InsertFirst(this);
        }
    }

    m_pDStreamOpenQueueMgr->LeaveCriticalSection();

    return m_pDStream;
};


// unlock the ILTStream for this item
void CDStreamOpenQueueItem::UnLockDStream()
{
    m_pDStreamOpenQueueMgr->EnterCriticalSection();

    // make sure counter is not already 0
    if (m_nLockCount == 0)
    {
        m_pDStreamOpenQueueMgr->LeaveCriticalSection();
        return;
    }

    // decrement lock count
    m_nLockCount--;

    // reduce any opened files over our maximum
    m_pDStreamOpenQueueMgr->ReduceOpenedItems();

    m_pDStreamOpenQueueMgr->LeaveCriticalSection();
};


// open the dstream
LTRESULT CDStreamOpenQueueItem::OpenDStream()
{
    m_pDStreamOpenQueueMgr->EnterCriticalSection();

    // make sure we have a file name
    if (m_DStreamFileRef_.m_pFilename == LTNULL) 
    {
        m_pDStreamOpenQueueMgr->LeaveCriticalSection();
        return LT_ERROR;
    }

    // check if the file is already opened
    if (m_pDStream != LTNULL) 
    {
        m_pDStreamOpenQueueMgr->LeaveCriticalSection();
        return LT_OK;
    }

    // set the file type
    m_DStreamFileRef_.m_FileType = FILE_ANYFILE;

    // open the file
    m_pDStream = client_file_mgr->OpenFile(&m_DStreamFileRef_);
    if (m_pDStream == LTNULL)
    {
        m_pDStreamOpenQueueMgr->LeaveCriticalSection();
        return LT_ERROR;
    }

    // seek to the position
    if (m_nSaveSeekPos != 0)
    {
        // seek to the saved position
        m_pDStream->SeekTo(m_nSaveSeekPos);
    }

    // since this is the most recently used put it at the top of the Opened list
    m_pDStreamOpenQueueMgr->m_lstClosedItems.Delete(this);
    m_pDStreamOpenQueueMgr->m_lstOpenedItems.InsertFirst(this);

    // reduce any opened files over our maximum
    m_pDStreamOpenQueueMgr->ReduceOpenedItems();

    m_pDStreamOpenQueueMgr->LeaveCriticalSection();

    return LT_OK;
};


// close the dstream
LTRESULT CDStreamOpenQueueItem::CloseDStream()
{
    m_pDStreamOpenQueueMgr->EnterCriticalSection();

    // check if the file is already closed
    if (m_pDStream == LTNULL) 
    {
        m_pDStreamOpenQueueMgr->LeaveCriticalSection();
        return LT_OK;
    }

    // save off our current position in the stream
    m_pDStream->GetPos(&m_nSaveSeekPos);

    // close the file
    m_pDStream->Release();
    m_pDStream = LTNULL;

    // move the file from the opened list to the closed list
    m_pDStreamOpenQueueMgr->m_lstOpenedItems.Delete(this);
    m_pDStreamOpenQueueMgr->m_lstClosedItems.Insert(this);

    m_pDStreamOpenQueueMgr->LeaveCriticalSection();

    return LT_OK;
};


// Initialize the queue with the specified number of items
void CDStreamOpenQueueMgr::Init(int nNumItems)
{
    // initialize the critical section 
    InitializeCriticalSection(&m_CriticalSection);

    // set the maximum number of open items we are targeting
    m_nMaxOpenedItems = nNumItems;
};


// Terminate the queue releasing all items
void CDStreamOpenQueueMgr::Term()
{
    // destroy all of the items
    DestroyAll();

    // delete the critical section 
    DeleteCriticalSection(&m_CriticalSection);
};


// Create a new item
CDStreamOpenQueueItem* CDStreamOpenQueueMgr::Create(const char* sFileName)
{
    // make sure file name is not null
    if (sFileName == LTNULL) return LTNULL;

    // create a new item
    CDStreamOpenQueueItem* pItem;
	LT_MEM_TRACK_ALLOC(pItem = new CDStreamOpenQueueItem(this),LT_MEM_TYPE_FILE);
    if (pItem == LTNULL) return LTNULL;

    // set the filename in the new item unless it was LTNULL
    if (sFileName != LTNULL) 
    {
        // assign the new file name to the item
        LT_MEM_TRACK_ALLOC(pItem->m_DStreamFileRef_.m_pFilename = pItem->file_name = new char[strlen(sFileName)+1],LT_MEM_TYPE_FILE);
        if (pItem->m_DStreamFileRef_.m_pFilename == LTNULL)
        {
            return LTNULL;
        }
        strcpy(pItem->file_name, sFileName);
    }
    else pItem->m_DStreamFileRef_.m_pFilename = LTNULL;

    // insert the new item into the closed list
    EnterCriticalSection();
    m_lstClosedItems.Insert(pItem);
    LeaveCriticalSection();

    return pItem;
};


// Destroy an item that was created
void CDStreamOpenQueueMgr::Destroy(CDStreamOpenQueueItem* pItem)
{
    // make sure the item is not null
    if (pItem == LTNULL) return;

    EnterCriticalSection();

    // is this an open item
    if (pItem->m_pDStream != LTNULL)
    {
        // close the item
        if (pItem->Close() != LT_OK)
        {
            // if we failed to close the item remove it from the open list
            m_lstOpenedItems.Delete(pItem);
        }
        else
        {
            // remove it from the closed list
            m_lstClosedItems.Delete(pItem);
        }
    }

    // if the item is already closed
    else
    {
        // remove it from the closed list
        m_lstClosedItems.Delete(pItem);
    }

    LeaveCriticalSection();

    // delete the item
    delete pItem;
};


// Destroy all items
void CDStreamOpenQueueMgr::DestroyAll()
{
    EnterCriticalSection();

    // close all items
    if (CloseAll() != LT_OK)
    {
        // go through closed list and destroy all items if they could not all be closed
        CDStreamOpenQueueItem* pItem = m_lstOpenedItems.GetFirst();
        while (pItem != LTNULL)
        {
            // destroy the item
            Destroy(pItem);

            // get first item again
            pItem = m_lstOpenedItems.GetFirst();
        }
    }

    // go through closed list and destroy all items
    CDStreamOpenQueueItem* pItem = m_lstClosedItems.GetFirst();
    while (pItem != LTNULL)
    {
        // destroy the item
        Destroy(pItem);

        // get first item again
        pItem = m_lstClosedItems.GetFirst();
    }

    LeaveCriticalSection();
};


// Destroy all items
LTRESULT CDStreamOpenQueueMgr::CloseAll()
{
    EnterCriticalSection();

    // go through closed list and destroy all items
    CDStreamOpenQueueItem* pItem = m_lstOpenedItems.GetFirst();
    while (pItem != LTNULL)
    {
        // close the item
        if (pItem->Close() != LT_OK) 
        {
            LeaveCriticalSection();
            return LT_ERROR;
        }

        // get next item
        pItem = pItem->Next();
    }

    LeaveCriticalSection();

    return LT_OK;
};


// reduce the number of opened files down to the max if possible
void CDStreamOpenQueueMgr::ReduceOpenedItems()
{
    EnterCriticalSection();
    
    // loop through opened items until we are not under the max opened or we have looked at them all
    // we must go through the list in reverse order because we want to get ride of the oldest items
    CDStreamOpenQueueItem* pItem = m_lstOpenedItems.GetLast();
    CDStreamOpenQueueItem* pPrevItem;
    while ((m_lstOpenedItems.GetNumItems() > m_nMaxOpenedItems) && (pItem != LTNULL))
    {
        // get the next item we will look at
        pPrevItem = pItem->Prev();

        // is this item not locked
        if (pItem->m_nLockCount == 0)
        {
            // close the item
            pItem->CloseDStream();
        }

        // go to next item
        pItem = pPrevItem;
    }

    LeaveCriticalSection();
};


