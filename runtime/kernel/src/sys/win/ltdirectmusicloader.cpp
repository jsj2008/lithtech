/****************************************************************************
;
;	MODULE:		LTDirectMusicLoader (.CPP)
;
;	PURPOSE:	Custom loader for DirectMusic to use DStreams for reading.
;
;	HISTORY:	1999 [blb]
;
;	NOTICE:		Copyright (c) 1999,2000 
;				Monolith Procutions, Inc.
;
***************************************************************************/


#include "bdefs.h"

#include "dmusici.h"

#include "lith.h"
#include "ltdirectmusicloader.h"

//server file mgr.
#include "client_filemgr.h"
static IClientFileMgr *g_pLTClientFileMgr;
define_holder(IClientFileMgr, g_pLTClientFileMgr);



// console output level for LTDirectMusic
// see ltdirectmusicloader_impl for functions and more information
#ifndef NOLITHTECH
extern int32 g_CV_LTDMConsoleOutput;
#endif

// output error to console
extern void LTDMConOutError(char *pMsg, ...);

// output warning to console
extern void LTDMConOutWarning(char *pMsg, ...);

// output message to console
extern void LTDMConOutMsg(int nLevel, char *pMsg, ...);


///////////////////////////////////////////////////////////////////////////////////////////
// macro for converting from normal to wide strings
///////////////////////////////////////////////////////////////////////////////////////////
#define MULTI_TO_WIDE( x,y )	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, y, -1, x, _MAX_PATH );


#ifdef USE_DSTREAM

////////////////////////////////////////////////////////////////////////////
// global variable to hold our CDStreamOpenQueueMgr object
////////////////////////////////////////////////////////////////////////////
CDStreamOpenQueueMgr g_LTDMDStreamOpenQueueMgr;

#endif


////////////////////////////////////////////////////////////////////////////
// class CLoadDirByClassItem
////////////////////////////////////////////////////////////////////////////
class CLoadDirByClassItem : public CBaseListItem
{
public:
	CLoadDirByClassItem() { m_guidClass = GUID_NULL; };

	// accessors for class
	REFCLSID GetClass() { return m_guidClass; };
	void SetClass(REFCLSID rguidClass) { memcpy((void*)&m_guidClass, (void*)&rguidClass, sizeof(CLSID)); };

	// accessors for path
	WCHAR* GetPath() { return m_pwzPath; };
	void SetPath(const WCHAR* pwzPath) { memset(m_pwzPath, '\0', MAX_PATH*sizeof(WCHAR)); wcsncpy(m_pwzPath, pwzPath, MAX_PATH-1); };

	// list operators from base class to get next and previous item in list
	CLoadDirByClassItem* Next() { return (CLoadDirByClassItem*)CBaseListItem::Next(); };
	CLoadDirByClassItem* Prev() { return (CLoadDirByClassItem*)CBaseListItem::Prev(); };

private:
	// Class id 
	CLSID m_guidClass;	

	// File path for directory
	WCHAR m_pwzPath[MAX_PATH]; 
};

////////////////////////////////////////////////////////////////////////////
// class CLoadDirByClassItem
////////////////////////////////////////////////////////////////////////////
class CLoadDirByClassList : public CLTBaseList
{
public:
	CLoadDirByClassList() { InitializeCriticalSection(&m_CriticalSection); };
	~CLoadDirByClassList() { RemoveAllItems(); DeleteCriticalSection(&m_CriticalSection); };

	// get the first item in the list
	CLoadDirByClassItem* GetFirst() { return (CLoadDirByClassItem*)CLTBaseList::GetFirst(); };

	// get the last item in the list
	CLoadDirByClassItem* GetLast() { return (CLoadDirByClassItem*)CLTBaseList::GetLast(); };

	// find the path item that contains the correct path for the given class id
	// if it can't find the requisted class ID it checks for the any class id
	CLoadDirByClassItem* Find(REFCLSID rguidClass, BOOL bCheckAllIfClassNotFound = TRUE);

	// add an item to the lits with the specified class and path
	void AddItem(REFCLSID rguidClass, const WCHAR* pwzPath);

	// remove and delete an item
	void RemoveItem(CLoadDirByClassItem* pItem);

	// remove and delete all items
	void RemoveAllItems();

	// helper functions to critical sections
	void EnterCriticalSection() { ::EnterCriticalSection(&m_CriticalSection); };
	void LeaveCriticalSection() { ::LeaveCriticalSection(&m_CriticalSection); };

private:

	// critical section to control access to this class from multiple threads
	CRITICAL_SECTION m_CriticalSection;
};


////////////////////////////////////////////////////////////////////////////
CLoadDirByClassItem* CLoadDirByClassList::Find(REFCLSID rguidClass, BOOL bCheckAllIfClassNotFound)
{
    EnterCriticalSection();

	// pointer to go through the list starting with the first item
	CLoadDirByClassItem* pCur = GetFirst();

	// loop through all items or until we find what we are looking for
	while (pCur != NULL)
	{
		// is this the item we are looking for return with it
		if (pCur->GetClass() == rguidClass) 
		{
			LeaveCriticalSection();
			return pCur;
		}

		// get the next item
		pCur = pCur->Next();
	}

	// if were were looking for the all types guid and didn't find it we should just exit 
	if (rguidClass == GUID_DirectMusicAllTypes)
	{
		LeaveCriticalSection();
		return NULL;
	}

	// if we are supposed to look for the all types class
	if (bCheckAllIfClassNotFound)
	{
		// since we didn't find our specific type just look for the all types guid
		pCur = Find(GUID_DirectMusicAllTypes);
		LeaveCriticalSection();
		return pCur;
	}
	else 
	{
		LeaveCriticalSection();
		return NULL;
	}
}


////////////////////////////////////////////////////////////////////////////
void CLoadDirByClassList::AddItem(REFCLSID rguidClass, const WCHAR* pwzPath)
{
	EnterCriticalSection();

	// get new item pointer exit if alloc failed
	CLoadDirByClassItem* pNew;
	LT_MEM_TRACK_ALLOC(pNew = new CLoadDirByClassItem,LT_MEM_TYPE_MUSIC);
	if (pNew == NULL) 
	{
		LeaveCriticalSection();
		return;
	}

	// set member values in new item
	pNew->SetClass(rguidClass);
	pNew->SetPath(pwzPath);

	// add the new item to the list
	Insert(pNew);

	LeaveCriticalSection();
}


////////////////////////////////////////////////////////////////////////////
void CLoadDirByClassList::RemoveItem(CLoadDirByClassItem* pItem)
{
	EnterCriticalSection();

	// remove the item from the list
	Delete(pItem);

	// delete the item from memory
	delete pItem;

	LeaveCriticalSection();
}


////////////////////////////////////////////////////////////////////////////
void CLoadDirByClassList::RemoveAllItems()
{
	EnterCriticalSection();

	// pointer to go through the list 
	CLoadDirByClassItem* pCur = GetFirst();

	// loop through all items deleting them
	while (pCur != NULL)
	{
		// remove the item
		RemoveItem(pCur);

		// get the first item again
		pCur = GetFirst();
	}

	LeaveCriticalSection();
}


////////////////////////////////////////////////////////////////////////////
// class CFileStreamItem
////////////////////////////////////////////////////////////////////////////
class CFileStreamItem : public CBaseListItem
{
public:

	// member variable access
	void SetFileStream(CLTDMFileStream* pFileStream) { m_pFileStream = pFileStream; };
	CLTDMFileStream* GetFileStream() { return m_pFileStream; };

	// list operators from base class to get next and previous item in list
	CFileStreamItem* Next() { return (CFileStreamItem*)CBaseListItem::Next(); };
	CFileStreamItem* Prev() { return (CFileStreamItem*)CBaseListItem::Prev(); };

private:
	CLTDMFileStream* m_pFileStream;

};


////////////////////////////////////////////////////////////////////////////
// class CLoadDirByClassItem
////////////////////////////////////////////////////////////////////////////
class CFileStreamList : public CLTBaseList
{
public:
	CFileStreamList() { InitializeCriticalSection(&m_CriticalSection); };
	~CFileStreamList() { RemoveAllItems(); DeleteCriticalSection(&m_CriticalSection); };

	// get the first item in the list
	CFileStreamItem* GetFirst() { return (CFileStreamItem*)CLTBaseList::GetFirst(); };

	// get the last item in the list
	CFileStreamItem* GetLast() { return (CFileStreamItem*)CLTBaseList::GetLast(); };

	// add an item to the list
	void AddItem(CLTDMFileStream* pFileStream);

	// remove an item from the list
	void DeleteItem(CLTDMFileStream* pFileStream);

	// remove and delete all items
	void RemoveAllItems();

	// helper functions to critical sections
	void EnterCriticalSection() { ::EnterCriticalSection(&m_CriticalSection); };
	void LeaveCriticalSection() { ::LeaveCriticalSection(&m_CriticalSection); };

private:

	// critical section to control access to this class from multiple threads
	CRITICAL_SECTION m_CriticalSection;
};


// add an item to the list
void CFileStreamList::AddItem(CLTDMFileStream* pFileStream)
{
	EnterCriticalSection();

	// allocate new item
	CFileStreamItem* pNewItem;
	LT_MEM_TRACK_ALLOC(pNewItem = new CFileStreamItem,LT_MEM_TYPE_MUSIC);
	if (pNewItem != NULL)
	{
		// set the member variable
		pNewItem->SetFileStream(pFileStream);

		/// insert into list
		Insert(pNewItem);
	}

	LeaveCriticalSection();
}


// remove an item from the list
void CFileStreamList::DeleteItem(CLTDMFileStream* pFileStream)
{
	EnterCriticalSection();

	// loop through all items
	CFileStreamItem* pFind = GetFirst();
	while (pFind != NULL)
	{
		// exit loop if we found the item
		if (pFind->GetFileStream() == pFileStream) break;

		// go to next item
		else pFind = pFind->Next();
	}
	// if we found the item
	if (pFind != NULL)
	{
		// delete from the list
		Delete(pFind);

		// delete from memory
		delete pFind;
	}

	LeaveCriticalSection();
}


////////////////////////////////////////////////////////////////////////////
void CFileStreamList::RemoveAllItems()
{
	EnterCriticalSection();

	// pointer to go through the list 
	CFileStreamItem* pCur = GetFirst();

	// loop through all items deleting them
	while (pCur != NULL)
	{
		// remove the item from memory
		delete pCur->GetFileStream();

		// get the first item again
		pCur = GetFirst();
	}

	LeaveCriticalSection();
}


/*  DirectMusic handles file names and object names in UNICODE. 
    This means you need to track whether your dll is running
    on a Win9x or WinNT machine, in order to make the right calls
    for file io. The global variable g_fIsUnicode should be initialized
    at startup with a call to LTDMsetUnicodeMode(). 
    In this way, your binary code runs on both Win9x and NT.
*/

static BOOL g_fIsUnicode;

void LTDMsetUnicodeMode()

{
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    g_fIsUnicode = 
		(osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS);
}


//-------------------------------------------------------------------------
// CLTDMFileStream::CFileBuffer
//-------------------------------------------------------------------------

CLTDMFileStream::CFileBuffer::CFileBuffer() :
	m_nRefCount(1),
	m_nFileSize(0),
	m_pFileData(NULL)
{
	m_pszFilename[0] = '\0';
}

CLTDMFileStream::CFileBuffer::~CFileBuffer()
{
	//sanity check
	assert(m_nRefCount == 0);

	//make sure to free our buffer
	Term();
}

//reference counting for this buffer
void CLTDMFileStream::CFileBuffer::AddRef()
{
	//add one to our reference count
	InterlockedIncrement(&m_nRefCount);

	LTDMConOutMsg(6, "LTDirectMusic CFileBuffer::AddRef file=%s Count=%d\n", m_pszFilename, m_nRefCount);
}

void CLTDMFileStream::CFileBuffer::Release()
{
	LTDMConOutMsg(6, "LTDirectMusic CFileBuffer::Release file=%s Count=%d\n", m_pszFilename, m_nRefCount - 1);

	if(InterlockedDecrement(&m_nRefCount) == 0)
	{
		LTDMConOutMsg(5, "LTDirectMusic freeing file %s\n", m_pszFilename);
		delete this;
	}
}

//releases all memory and invalidates the object
void CLTDMFileStream::CFileBuffer::Term()
{
	delete [] m_pFileData;
	m_pFileData = NULL;

	m_nFileSize = 0;
	m_pszFilename[0] = '\0';
}

//ensures that the file is available for any read or seek operations
bool CLTDMFileStream::CFileBuffer::EnsureFileOpen()
{
	//see if we are already open
	if(m_pFileData)
		return true;

	//we aren't, so try and open
	if(m_pszFilename[0] == '\0')
		return false;
	
	LTDMConOutMsg(5, "LTDirectMusic loading file %s\n", m_pszFilename);

	//fill out a file reference for this file
	FileRef Ref;
	Ref.m_FileType = FILE_CLIENTFILE;
	Ref.m_pFilename = m_pszFilename;
	
	//try and open it
	ILTStream* pStream = g_pLTClientFileMgr->OpenFile(&Ref);

	if(!pStream)
	{
		LTDMConOutWarning("LTDirectMusic loader failed to open file %s\n", m_pszFilename);
		Term();
		return false;
	}

	//alright, we have the file, now lets allocate a buffer for it
	m_nFileSize = pStream->GetLen();

	LT_MEM_TRACK_ALLOC(m_pFileData = new uint8[m_nFileSize], LT_MEM_TYPE_MUSIC);

	//check the allocation
	if(!m_pFileData)
	{
		LTDMConOutWarning("LTDirectMusic loader failed to allocate buffer for file %s\n", m_pszFilename);
		Term();

		pStream->Release();
		return false;
	}

	//now load the file into the buffer
	if(pStream->Read(m_pFileData, m_nFileSize) != LT_OK)
	{
		LTDMConOutWarning("LTDirectMusic loader failed to read buffer for file %s\n", m_pszFilename);
		Term();

		pStream->Release();
		return false;
	}

	//we are done with the stream
	pStream->Release();

	return true;
}

//-------------------------------------------------------------------------
// CLTDMFileStream
//-------------------------------------------------------------------------

CLTDMFileStream::CLTDMFileStream( CLTDMLoader *pLoader)

{
	// Start with one reference for caller.
	m_cRef				= 1;        
	
#ifdef LTDM_BUFFER_MUSIC_FILES

	m_pBuffer			= NULL;
	m_nFileOffset		= 0;

#else

	m_pOpenQueueItem = NULL;

#endif

	// Link to loader, so loader can be found from stream.
	m_pLoader = pLoader; 
	if (pLoader)
	{
		pLoader->AddRefP(); // Addref the private counter to avoid cyclic references.

		// add to list of file streams
		pLoader->GetFileStreamList()->AddItem(this);
	}
}

CLTDMFileStream::~CLTDMFileStream() 

{ 
	Close();

	if (m_pLoader)
	{
		// remove from list of file streams
		m_pLoader->GetFileStreamList()->DeleteItem(this);

		m_pLoader->ReleaseP();
	}
}

const char* CLTDMFileStream::GetFilename() const
{
#ifdef LTDM_BUFFER_MUSICFILES

	if(m_pBuffer)
		return m_pBuffer->m_pszFilename;

#else

	if(m_pOpenQueueItem)
		return m_pOpenQueueItem->GetFileName();

#endif

	return "Error";
}

HRESULT CLTDMFileStream::Open(WCHAR * lpFileName,unsigned long dwDesiredAccess)
{
	//make sure we don't have any files already open
	Close();

	// make sure file name is not null
	if (lpFileName == NULL) 
	{
		return E_FAIL;
	}

#ifdef LTDM_BUFFER_MUSIC_FILES

	//allocate a new buffer
	LT_MEM_TRACK_ALLOC(m_pBuffer = new CFileBuffer, LT_MEM_TYPE_MUSIC);

	if(!m_pBuffer)
	{
		Close();
		return E_FAIL;
	}

	// convert file name to regular characters
	wcstombs( m_pBuffer->m_pszFilename, lpFileName, sizeof(m_pBuffer->m_pszFilename) );

	//reset our file pointer
	m_nFileOffset = 0;

#else

	// create an item in the OpenQueueMgr
	char sFileName[MAX_PATH + 1];
	wcstombs( sFileName, lpFileName, MAX_PATH + 1 );
	m_pOpenQueueItem = g_LTDMDStreamOpenQueueMgr.Create(sFileName);

	if (m_pOpenQueueItem == NULL)
	{
		return E_FAIL;
	}

	// open the file
	if (m_pOpenQueueItem->Open() != LT_OK)
	{
		LTDMConOutWarning("LTDirectMusic loader failed to open file %s\n", sFileName);

		// destroy the OpenQueue item
		g_LTDMDStreamOpenQueueMgr.Destroy(m_pOpenQueueItem);
		m_pOpenQueueItem = NULL;

		return E_FAIL;
	}

	// make sure we are at the start of the file
	// there is no reason to do this since we just opened the file

	LTDMConOutMsg(5, "CLTDMFileStream::Open FileName=%s\n", sFileName);

#endif

	//success
	return S_OK;
}


HRESULT CLTDMFileStream::Close()
{

#ifdef LTDM_BUFFER_MUSIC_FILES

	//clean up our resources
	if(m_pBuffer)
	{
		m_pBuffer->Release();
		m_pBuffer = NULL;
	}

	m_nFileOffset = 0;

#else

	if (m_pOpenQueueItem != NULL)
	{
		m_pOpenQueueItem->Close();
		g_LTDMDStreamOpenQueueMgr.Destroy(m_pOpenQueueItem);
		m_pOpenQueueItem = NULL;
	}

#endif

	return S_OK;
}

STDMETHODIMP CLTDMFileStream::QueryInterface( const IID &riid, void **ppvObj )
{
    if (riid == IID_IUnknown || riid == IID_IStream) {
        *ppvObj = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    }
	else if (riid == IID_IDirectMusicGetLoader8) 
    {
        *ppvObj = static_cast<IDirectMusicGetLoader8*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObj = NULL;
    return E_NOINTERFACE;
}


/*  The GetLoader interface is used to find the loader from the IStream.
    When an object is loading data from the IStream via the object's
    IPersistStream interface, it may come across a reference chunk that
    references another object that also needs to be loaded. It QI's the
    IStream for the IDirectMusicGetLoader interface. It then uses this
    interface to call GetLoader and get the actual loader. Then, it can
    call GetObject on the loader to load the referenced object.
*/

STDMETHODIMP CLTDMFileStream::GetLoader(
	IDirectMusicLoader ** ppLoader)	// Returns an AddRef'd pointer to the loader.

{
	if (m_pLoader)
	{
		return m_pLoader->QueryInterface( IID_IDirectMusicLoader8,(void **) ppLoader );
	}
	*ppLoader = NULL;
	return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CLTDMFileStream::AddRef()
{
	LTDMConOutMsg(6, "CLTDMFileStream::AddRef %s RefCount=%d\n", GetFilename(), m_cRef + 1);
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CLTDMFileStream::Release()
{
	LTDMConOutMsg(6, "CLTDMFileStream::Release %s RefCount=%d\n", GetFilename(), m_cRef - 1);
    if (!InterlockedDecrement(&m_cRef)) 
	{
		delete this;
		return 0;
    }
    return m_cRef;
}


/* IStream methods */
STDMETHODIMP CLTDMFileStream::Read( void* pv, ULONG cb, ULONG* pcbRead )
{
	LTDMConOutMsg(7, "CLTDMFileStream::Read %s Size=%d\n", GetFilename(), cb);

#ifdef LTDM_BUFFER_MUSIC_FILES

	if(!m_pBuffer || !m_pBuffer->EnsureFileOpen())
		return E_FAIL;

	//determine the maximum amount of file we can load
	uint32 nCopyAmount = cb;

	if((m_nFileOffset + cb) > m_pBuffer->GetFileSize())
		nCopyAmount = m_pBuffer->GetFileSize() - m_nFileOffset;

	//copy that file over
	memcpy(pv, m_pBuffer->GetFileData() + m_nFileOffset, nCopyAmount);

	//let the callee know how much we copied
	if(pcbRead)
		*pcbRead = nCopyAmount;

	//adjust our file pointer
	m_nFileOffset += nCopyAmount;

#else

	// make sure our file item pointer is valid
	if (m_pOpenQueueItem == NULL) 
	{
		return E_FAIL;
	}

	// make sure the file is open and get a pointer to the ILTStream
	ILTStream* pLTStream = m_pOpenQueueItem->LockDStream();
	if (pLTStream == LTNULL)
	{
		return E_FAIL;
	}

	// read in the data
	if (pLTStream->Read( pv, cb) != LT_OK)
	{
		m_pOpenQueueItem->UnLockDStream();
		return E_FAIL;
	}

	// set the number of bytes read
	if (pcbRead != NULL) *pcbRead = cb;

	m_pOpenQueueItem->UnLockDStream();

#endif

	//success
	return S_OK;
}

STDMETHODIMP CLTDMFileStream::Write( const void* pv, ULONG cb, ULONG* pcbWritten )
{
	return E_FAIL;
}

STDMETHODIMP CLTDMFileStream::Seek( LARGE_INTEGER dlibMove, unsigned long dwOrigin, ULARGE_INTEGER* plibNewPosition )
{
	LTDMConOutMsg(7, "CLTDMFileStream::Seek %s", GetFilename());

#ifdef LTDM_BUFFER_MUSIC_FILES

	if(!m_pBuffer || !m_pBuffer->EnsureFileOpen())
		return E_FAIL;

	uint32 nSeekFrom = 0;

	switch(dwOrigin)
	{
	case SEEK_CUR:
		nSeekFrom = m_nFileOffset;
		break;
	case SEEK_END:
		nSeekFrom = m_pBuffer->GetFileSize();
		break;
	case SEEK_SET:
		nSeekFrom = 0;
		break;
	default:
		return E_FAIL;
		break;
	}

	//alright, now compute our final offset
	int32 nFinalOffset = (int32)nSeekFrom + (int32)dlibMove.LowPart;

	if((nFinalOffset < 0) || ((uint32)nFinalOffset > m_pBuffer->GetFileSize()))
		return E_FAIL;

	//seek to that position
	m_nFileOffset = (uint32)nFinalOffset;

	//move it back into the output
	if(plibNewPosition)
	{
		plibNewPosition->LowPart  = m_nFileOffset;
		plibNewPosition->HighPart = 0;
	}

#else

	// make sure our file item pointer is valid
	if (m_pOpenQueueItem == NULL) 
	{
		return E_FAIL;
	}

	// make sure the file is open and get a pointer to the ILTStream
	ILTStream* pLTStream = m_pOpenQueueItem->LockDStream();
	if (pLTStream == LTNULL)
	{
		return E_FAIL;
	}

	// current position in an item
	DWORD nSeekPos;
	
	// figure out the seek position relative to the origin
	if (dwOrigin == SEEK_SET)
	{
		nSeekPos =  dlibMove.LowPart;
	}
	else
	{
		// relative to current position
		if (dwOrigin == SEEK_CUR)
		{
			LARGE_INTEGER nCurPos;
			if (pLTStream->GetPos((uint32*)&nCurPos.LowPart) != LT_OK) 
			{
				m_pOpenQueueItem->UnLockDStream();
				return E_FAIL;
			}
			nCurPos.HighPart = 0;
			
			LARGE_INTEGER nNewPos;
			nNewPos.QuadPart = nCurPos.QuadPart + dlibMove.QuadPart;

			nSeekPos = nNewPos.LowPart;

			// we don't need to seek if we didn't ask to move anywhere
			if (dlibMove.QuadPart == 0) 
			{
				// set the return current position if it was not NULL
				if( plibNewPosition != NULL )
				{
					plibNewPosition->QuadPart = nCurPos.QuadPart;
				}

				m_pOpenQueueItem->UnLockDStream();
				return S_OK;
			}
		}
		else
		{
			if (dwOrigin == SEEK_END)
			{
				LARGE_INTEGER nSize;
				pLTStream->GetLen((uint32*)&nSize.LowPart);
				nSize.HighPart = 0;

				LARGE_INTEGER nNewPos;
				nNewPos.QuadPart = nSize.QuadPart + dlibMove.QuadPart;

				nSeekPos = nNewPos.LowPart;
			}
			else
			{
				// unknown origin we must fail
				m_pOpenQueueItem->UnLockDStream();
				return E_FAIL;
			}
		}
	}

	// do the seek
	if (pLTStream->SeekTo(nSeekPos) != LT_OK)
	{
		// return fail if we failed
		m_pOpenQueueItem->UnLockDStream();
		return E_FAIL;
	}

	// set the return current position if it was not NULL
	if( plibNewPosition != NULL )
	{
		pLTStream->GetPos((uint32*)&plibNewPosition->LowPart);
		if( dlibMove.LowPart < 0 )
		{
			plibNewPosition->HighPart = (uint32)-1;
		}
		else
		{
			plibNewPosition->HighPart = 0;
		}
	}

	// everything worked so we return successful
	m_pOpenQueueItem->UnLockDStream();

#endif

	return S_OK;
}


STDMETHODIMP CLTDMFileStream::SetSize( ULARGE_INTEGER /*libNewSize*/ )
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMFileStream::CopyTo( IStream* /*pstm */, ULARGE_INTEGER /*cb*/,
                     ULARGE_INTEGER* /*pcbRead*/,
                     ULARGE_INTEGER* /*pcbWritten*/ )
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMFileStream::Commit( unsigned long /*grfCommitFlags*/ )
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMFileStream::Revert()
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMFileStream::LockRegion( ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/,
                         unsigned long /*dwLockType*/ )
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMFileStream::UnlockRegion( ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/,
                           unsigned long /*dwLockType*/)
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMFileStream::Stat( STATSTG* /*pstatstg*/, unsigned long /*grfStatFlag*/ )
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMFileStream::Clone( IStream** ppstm )
{ 
	LTDMConOutMsg(6, "LTDirectMusic cloning file %s\n", GetFilename());

#ifdef LTDM_BUFFER_MUSIC_FILES

	//we can't clone a file without a buffer
	if(!m_pBuffer)
	{
		return E_FAIL;
	}

	// allocate a new stream object
	CLTDMFileStream* pStream;
	LT_MEM_TRACK_ALLOC(pStream = new CLTDMFileStream( m_pLoader ),LT_MEM_TYPE_MUSIC);

	if ( !pStream )
		return E_FAIL;

	//make sure to copy over the buffer
	pStream->m_pBuffer = m_pBuffer;
	pStream->m_pBuffer->AddRef();

	pStream->m_nFileOffset	= m_nFileOffset;

	// get the IStream interface
	if(ppstm)
	{
		*ppstm = static_cast<IStream*>(pStream);
	}

#else

	HRESULT hr;

	// allocate a new stream object
	CLTDMFileStream* pStream;
	LT_MEM_TRACK_ALLOC(pStream = new CLTDMFileStream( m_pLoader ),LT_MEM_TYPE_MUSIC);

	if ( !pStream )
		return E_FAIL;

	// get the IStream interface
	hr = pStream->QueryInterface( IID_IStream, (void**) ppstm );	

	// if everything is OK, set up the seek position
	if ( SUCCEEDED( hr ) )
	{
		pStream->m_pLoader = this->m_pLoader;

		// add a new queue item, identical to the current one
		CDStreamOpenQueueItem* pOpenQueueItem = g_LTDMDStreamOpenQueueMgr.Create(m_pOpenQueueItem->GetFileName());

		if ( !pOpenQueueItem )
			return E_FAIL;

		pStream->m_pOpenQueueItem = pOpenQueueItem;

		// now set the seek parameter

		// make sure the file is open and get a pointer to the ILTStream
		ILTStream* pLTStream = pOpenQueueItem->LockDStream();

		if (pLTStream == LTNULL)
		{
			return E_FAIL;
		}

		// current position in an item
		DWORD nSeekPos;
		LARGE_INTEGER nCurPos;
		if (pLTStream->GetPos((uint32*)&nCurPos.LowPart) != LT_OK) 
		{
			pOpenQueueItem->UnLockDStream();
			return E_FAIL;
		}
		nCurPos.HighPart = 0;
		
		LARGE_INTEGER nNewPos;
		nNewPos.QuadPart = nCurPos.QuadPart;

		nSeekPos = nNewPos.LowPart;

		// do the seek
		if (pLTStream->SeekTo(nSeekPos) != LT_OK)
		{		
			// return fail if we failed
			pOpenQueueItem->UnLockDStream();
			return E_FAIL;
		}

		pOpenQueueItem->UnLockDStream();
	}

#endif

	return S_OK; 
}


CLTDMMemStream::CLTDMMemStream( CLTDMLoader *pLoader)

{
	m_cRef = 1;
	m_pbData = NULL;
	m_llLength = 0;
	m_llPosition = 0;
	m_pLoader = pLoader;
	if (pLoader)
	{
		pLoader->AddRefP();
	}
}

CLTDMMemStream::~CLTDMMemStream() 

{ 
	if (m_pLoader)
	{
		m_pLoader->ReleaseP();
	}
	Close();
}

HRESULT CLTDMMemStream::Open(BYTE *pbData, LONGLONG llLength)

{
	Close();
	m_pbData = pbData;
	m_llLength = llLength;
	m_llPosition = 0;
	if ((pbData == NULL) || (llLength == 0))
	{
		return DMUS_E_LOADER_FAILEDOPEN;
	}
	if (IsBadReadPtr(pbData, (DWORD) llLength))
	{
		m_pbData = NULL;
		m_llLength = 0;
		return DMUS_E_LOADER_FAILEDOPEN;
	}
	return S_OK;
}

HRESULT CLTDMMemStream::Close()

{
	m_pbData = NULL;
	m_llLength = 0;
	return S_OK;
}

STDMETHODIMP CLTDMMemStream::QueryInterface( const IID &riid, void **ppvObj )
{
    if (riid == IID_IUnknown || riid == IID_IStream) {
        *ppvObj = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    }
	else if (riid == IID_IDirectMusicGetLoader8) 
    {
        *ppvObj = static_cast<IDirectMusicGetLoader8*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObj = NULL;
    return E_NOINTERFACE;
}


STDMETHODIMP CLTDMMemStream::GetLoader(
	IDirectMusicLoader ** ppLoader)	

{
	if (m_pLoader)
	{
		return m_pLoader->QueryInterface( IID_IDirectMusicLoader8,(void **) ppLoader );
	}
	*ppLoader = NULL;
	return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CLTDMMemStream::AddRef()
{
	LTDMConOutMsg(6, "CLTDMMemStream::AddRef RefCount=%d\n", m_cRef + 1);
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CLTDMMemStream::Release()
{
	LTDMConOutMsg(6, "CLTDMMemStream::Release RefCount=%d\n", m_cRef - 1);

    if (!InterlockedDecrement(&m_cRef)) 
	{
		delete this;
		return 0;
    }
    return m_cRef;
}

/* IStream methods */
STDMETHODIMP CLTDMMemStream::Read( void* pv, ULONG cb, ULONG* pcbRead )
{
	LTDMConOutMsg(7, "CLTDMMemStream::Read Size=%d\n", cb);

	if ((cb + m_llPosition) <= m_llLength)
	{
		memcpy(pv,&m_pbData[m_llPosition],cb);
		m_llPosition += cb;
		if( pcbRead != NULL )
		{
			*pcbRead = cb;
		}
		return S_OK;
	}
	return E_FAIL ;
}

STDMETHODIMP CLTDMMemStream::Write( const void* pv, ULONG cb, ULONG* pcbWritten )
{
    return E_NOTIMPL;
}

STDMETHODIMP CLTDMMemStream::Seek( LARGE_INTEGER dlibMove, unsigned long dwOrigin, ULARGE_INTEGER* plibNewPosition )
{
	// Since we only parse RIFF data, we can't have a file over 
	// DWORD in length, so disregard high part of LARGE_INTEGER.
	LTDMConOutMsg(7, "CLTDMMemStream::Seek\n");

	LONGLONG llOffset;

	llOffset = dlibMove.QuadPart;
	if (dwOrigin == STREAM_SEEK_CUR)
	{
		llOffset += m_llPosition;
	} 
	else if (dwOrigin == STREAM_SEEK_END)
	{
		llOffset += m_llLength;
	}
	if ((llOffset >= 0) && (llOffset <= m_llLength))
	{
		m_llPosition = llOffset;
	}
	else return E_FAIL;

	if( plibNewPosition != NULL )
	{
		plibNewPosition->QuadPart = m_llPosition;
	}
    return S_OK;
}

STDMETHODIMP CLTDMMemStream::SetSize( ULARGE_INTEGER /*libNewSize*/ )
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMMemStream::CopyTo( IStream* /*pstm */, ULARGE_INTEGER /*cb*/,
                     ULARGE_INTEGER* /*pcbRead*/,
                     ULARGE_INTEGER* /*pcbWritten*/ )
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMMemStream::Commit( unsigned long /*grfCommitFlags*/ )
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMMemStream::Revert()
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMMemStream::LockRegion( ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/,
                         unsigned long /*dwLockType*/ )
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMMemStream::UnlockRegion( ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/,
                           unsigned long /*dwLockType*/)
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMMemStream::Stat( STATSTG* /*pstatstg*/, unsigned long /*grfStatFlag*/ )
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CLTDMMemStream::Clone( IStream** /*ppstm*/ )
{ 
	return E_NOTIMPL; 
}


CLTDMLoader::CLTDMLoader()

{
    InitializeCriticalSection(&m_CriticalSection);
#ifdef USE_REZMGR
	InitializeCriticalSection(&m_RezMgrCriticalSection);
#endif
	m_cRef = 1;
	m_cPRef = 0;
    m_pObjectList = NULL;
    MULTI_TO_WIDE(m_wszForwardSlash, "/");
	MULTI_TO_WIDE(m_wszBackSlash, "\\");
	LT_MEM_TRACK_ALLOC(m_pLoadDirByClassList = new CLoadDirByClassList,LT_MEM_TYPE_MUSIC);
	LT_MEM_TRACK_ALLOC(m_pFileStreamList = new CFileStreamList,LT_MEM_TYPE_MUSIC);
#ifdef USE_DSTREAM
	g_LTDMDStreamOpenQueueMgr.Init(5); // 5 simultaneous ILTStream accesses allowed
#endif
}

CLTDMLoader::~CLTDMLoader()

{
    ClearObjectList();

#ifdef USE_REZMGR
	// close the rezmgr if it is open 
	if (m_RezMgr.IsOpen()) m_RezMgr.Close();
#endif
	if (m_pLoadDirByClassList != NULL) 
	{
		delete m_pLoadDirByClassList;
		m_pLoadDirByClassList = NULL;
	}
	if (m_pFileStreamList != NULL)
	{
		delete m_pFileStreamList;
		m_pFileStreamList = NULL;
	}
#ifdef USE_DSTREAM
	g_LTDMDStreamOpenQueueMgr.Term();
#endif
    DeleteCriticalSection(&m_CriticalSection);
#ifdef USE_REZMGR
	DeleteCriticalSection(&m_RezMgrCriticalSection);
#endif
}


HRESULT CLTDMLoader::Init()

{
    HRESULT hr = S_OK;

    // Initialize loader to handle Unicode properly for this machine. 
    LTDMsetUnicodeMode();

    return hr;
}

void CLTDMLoader::ClearObjectList()
{
 	// used at end of level to get rid of references to objects
	while (m_pObjectList)
    {
        CLTDMObjectRef * pObject = m_pObjectList;
        m_pObjectList = pObject->m_pNext;

        if (pObject->m_pObject)
        {
            pObject->m_pObject->Release();
        }
        delete pObject;
    }
}

// CLTDMLoader::QueryInterface
//
STDMETHODIMP
CLTDMLoader::QueryInterface(const IID &iid,
                                   void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IDirectMusicLoader8) {
        *ppv = static_cast<IDirectMusicLoader8*>(this);
    }
	else 
	{
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


// CLTDMLoader::AddRef
//
STDMETHODIMP_(ULONG)
CLTDMLoader::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CLTDMLoader::AddRefP()
{
    return InterlockedIncrement(&m_cPRef);
}

// CLTDMLoader::Release
//
STDMETHODIMP_(ULONG)
CLTDMLoader::Release()
{
    if (!InterlockedDecrement(&m_cRef)) 
	{
		InterlockedIncrement(&m_cRef);		// Keep streams from deleting loader.
		ClearCache(GUID_DirectMusicAllTypes);
		if (!InterlockedDecrement(&m_cRef))
		{
			if (!m_cPRef)
			{
				delete this;
				return 0;
			}
		}
    }
    return m_cRef;
}

ULONG CLTDMLoader::ReleaseP()
{
    if (!InterlockedDecrement(&m_cPRef)) 
	{
		if (!m_cRef)
		{
			delete this;
			return 0;
		}
    }
    return m_cPRef;
}

STDMETHODIMP CLTDMLoader::GetObject(
	LPDMUS_OBJECTDESC pDESC,	// Description of the requested object in <t DMUS_OBJECTDESC> structure.
    REFIID riid,                // The interface type to return in <p ppv>
	LPVOID FAR *ppv)	        // Receives the interface on success.

{
	HRESULT hr = E_NOTIMPL;

    EnterCriticalSection(&m_CriticalSection);
    IDirectMusicObject * pIObject;
    // At this point, the loader should check with all the objects it already
    // has loaded. It should look for file name, object guid, and name.
    // In this case, we are being cheap and looking for only the object
    // guid, which should be guaranteed to be unique. If the files are all
    // created with guids, (and producer supports that) this should work
    // well.
    // If it sees that the object is already loaded, it should
    // return a pointer to that one and increment the reference. 
    // It is very important to keep the previously loaded objects
    // "cached" in this way. Otherwise, objects, like DLS collections, will get loaded
    // multiple times with a very great expense in memory and efficiency!
    // This is primarily an issue when object reference each other. For
    // example, segments reference style and collection objects. 
    if (pDESC->dwValidData & DMUS_OBJ_OBJECT)
    {
        CLTDMObjectRef * pObject = NULL;
        for (pObject = m_pObjectList;pObject;pObject = pObject->m_pNext)
        {
            if (pDESC->guidObject == pObject->m_guidObject)
            {
                break;
            }
        }
        if (pObject)
        {
            hr = E_FAIL;
            pIObject = pObject->m_pObject;
            if (pObject->m_pObject)
            {
                hr = pObject->m_pObject->QueryInterface( riid, ppv );
            }
            LeaveCriticalSection(&m_CriticalSection);
	        return hr;
        }
    }

    if (pDESC->dwValidData & DMUS_OBJ_FILENAME)
	{
		hr = LoadFromFile(pDESC,&pIObject);
	}
	else if (pDESC->dwValidData & DMUS_OBJ_MEMORY)
	{
		hr = LoadFromMemory(pDESC,&pIObject);
	}
	else 
		hr = DMUS_E_LOADER_NOFILENAME;

    if (SUCCEEDED(hr))
    {
        // If we succeeded in loading it, keep a pointer to it, 
        // addref it, and keep the guid for finding it next time.
        // To get the GUID, call ParseDescriptor on the object and
        // it will fill in the fields it knows about, including the
        // guid.
        DMUS_OBJECTDESC DESC;
		memset((void *)&DESC,0,sizeof(DESC));
		DESC.dwSize = sizeof (DMUS_OBJECTDESC);	
		pIObject->GetDescriptor(&DESC);

        if (DESC.dwValidData & DMUS_OBJ_OBJECT)
        {
            CLTDMObjectRef * pObject;
			LT_MEM_TRACK_ALLOC(pObject = new CLTDMObjectRef,LT_MEM_TYPE_MUSIC);
            if (pObject)
            {
                pObject->m_guidObject = DESC.guidObject;
                pObject->m_pNext = m_pObjectList;
                m_pObjectList = pObject;
                pObject->m_pObject = pIObject;
                pIObject->AddRef();
            }
        }
        hr = pIObject->QueryInterface( riid, ppv );
        pIObject->Release();

    }
	LeaveCriticalSection(&m_CriticalSection);
	return hr;
}

HRESULT CLTDMLoader::LoadFromFile(LPDMUS_OBJECTDESC pDesc,IDirectMusicObject8 ** ppIObject)

{
	HRESULT hr;
    
	hr = CoCreateInstance(pDesc->guidClass,
		NULL,CLSCTX_INPROC_SERVER,IID_IDirectMusicObject8,
		(void **) ppIObject);

	if (SUCCEEDED(hr))
	{
		WCHAR wzFullPath[DMUS_MAX_FILENAME];
		ZeroMemory( wzFullPath, sizeof(WCHAR) * DMUS_MAX_FILENAME );

		CLTDMFileStream *pStream;
		LT_MEM_TRACK_ALLOC(pStream = new CLTDMFileStream ( this ),LT_MEM_TYPE_MUSIC);

		if (pStream)
		{
			if (pDesc->dwValidData & DMUS_OBJ_FULLPATH)
			{
				wcscpy(wzFullPath,pDesc->wszFileName);
			}
			else
			{
				// find the directory that matches this class and copy it to the path
				CLoadDirByClassItem* pItem = m_pLoadDirByClassList->Find(pDesc->guidClass);
				if (pItem != NULL) wcscpy(wzFullPath, pItem->GetPath());

				// check if slash was on end of string if not add it
				if ((wzFullPath[wcslen(wzFullPath)-1] != m_wszBackSlash[0]) && 
					(wzFullPath[wcslen(wzFullPath)-1] != m_wszForwardSlash[0]))
				{
					wcscat(wzFullPath, m_wszBackSlash);
				}

				// add filename to string
				wcscat(wzFullPath, pDesc->wszFileName);
			}
			
			hr = pStream->Open(wzFullPath,GENERIC_READ);
			if (SUCCEEDED(hr))
			{
				IPersistStream* pIPS;
				hr = (*ppIObject)->QueryInterface( IID_IPersistStream, (void**)&pIPS );

				if (SUCCEEDED(hr))
				{
                    // Now that we have the IPersistStream interface from the object, we can ask it to load from our stream!
					hr = pIPS->Load( pStream );
					pIPS->Release();
				}
			}

			pStream->Release();
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}


        if (FAILED(hr))
        {
       		(*ppIObject)->Release();
        }
	}
	return hr;
}

HRESULT CLTDMLoader::LoadFromMemory(LPDMUS_OBJECTDESC pDesc,IDirectMusicObject8 ** ppIObject)

{
	HRESULT hr;
	hr = CoCreateInstance(pDesc->guidClass,
		NULL,CLSCTX_INPROC_SERVER,IID_IDirectMusicObject8,
		(void **) ppIObject);
	if (SUCCEEDED(hr))
	{
		CLTDMMemStream *pStream;
		LT_MEM_TRACK_ALLOC(pStream = new CLTDMMemStream ( this ),LT_MEM_TYPE_MUSIC);
		if (pStream)
		{
			hr = pStream->Open(pDesc->pbMemData,pDesc->llMemLength);
			if (SUCCEEDED(hr))
			{
				IPersistStream* pIPS;
				hr = (*ppIObject)->QueryInterface( IID_IPersistStream, (void**)&pIPS );
				if (SUCCEEDED(hr))
				{
                    // Now that we have the IPersistStream interface from the object, we can ask it to load from our stream!
					hr = pIPS->Load( pStream );
					pIPS->Release();
				}
			}
			pStream->Release();
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
        if (FAILED(hr))
        {
       		(*ppIObject)->Release();
        }
	}
	return hr;
}


STDMETHODIMP CLTDMLoader::SetObject(
	LPDMUS_OBJECTDESC pDESC)

{
	HRESULT hr = E_NOTIMPL;
    EnterCriticalSection(&m_CriticalSection);
	LeaveCriticalSection(&m_CriticalSection);
 	return hr;
}


STDMETHODIMP CLTDMLoader::SetSearchDirectory(
	REFCLSID rguidClass,	// Class id identifies which class of objects this pertains to.
					        // Optionally, GUID_DirectMusicAllTypes specifies all classes. 
	WCHAR *pwzPath,         // File path for directory. Must be a valid directory and
					        // must be less than MAX_PATH in length.
	BOOL fClear)	        // If TRUE, clears all information about objects
					        // prior to setting directory. 
					        // This helps avoid accessing objects from the
					        // previous directory that may have the same name.
					        // However, this will not remove cached objects.
										
{
	// see if there is already a directory set for this class
	CLoadDirByClassItem* pItem = m_pLoadDirByClassList->Find(rguidClass, FALSE);
	if (pItem != NULL)
	{
		// if path passed in is NULL we should remove the item
		if (pwzPath == NULL)
		{
			m_pLoadDirByClassList->RemoveItem(pItem);
		}

		// just set our new path to the item
		else
		{
			pItem->SetPath(pwzPath);
		}
	}

	// create a new item
	else
	{
		// add the new item with the specified 
		m_pLoadDirByClassList->AddItem(rguidClass, pwzPath);
	}

    return S_OK;
}


// same as regular setsearch directory but takes regular strings
STDMETHODIMP CLTDMLoader::SetSearchDirectory(REFCLSID rguidClass, const char* sPath, BOOL fClear)
{
	// fail if string is too long
	if (strlen(sPath) >= MAX_PATH) return E_FAIL;

	// string to hold new wide string
	WCHAR wszPath[MAX_PATH];

	// convert string
    MULTI_TO_WIDE( wszPath, sPath );

	// call the real SetSearchDirectory
	return SetSearchDirectory(rguidClass, wszPath, fClear);
}


#ifdef USE_REZMGR
STDMETHODIMP CLTDMLoader::SetRezFile(const char* sRezName)
{
	EnterRezMgrCriticalSection();

	// if the name is NULL then close the rez file
	if (sRezName == NULL)
	{
		// close the file if it is open are return success
		if (m_RezMgr.IsOpen()) m_RezMgr.Close();
		LeaveRezMgrCriticalSection();
		return S_OK;
	}

	// convert file name to regular string
	if ((strlen(sRezName) > 0) && (strlen(sRezName) < MAX_PATH))
	{
		char sFileName[MAX_PATH + 1];
		LTStrCpy(sFileName, sRezName, sizeof(sFileName));

		// remove trailing slash if any
		if ((sFileName[strlen(sFileName)-1] == '\\') ||
			(sFileName[strlen(sFileName)-1] == '/'))
		{
			sFileName[strlen(sFileName)-1] = '\0';
		}

		// if the RezMgr is already open 
		if (m_RezMgr.IsOpen())
		{
			// open an additional rez file
			m_RezMgr.OpenAdditional(sFileName, TRUE);
		}

		// if it was not open then open it
		else 
		{
			// open the control file
			if (!m_RezMgr.Open(sFileName))
			{
				// if we failed then return error
				LeaveRezMgrCriticalSection();
				return E_FAIL;
			}
		}
	}

	// if file name contained nothing return error
	else 
	{
		LeaveRezMgrCriticalSection();
		return E_FAIL;
	}

	LeaveRezMgrCriticalSection();
	return S_OK;
}
#endif


STDMETHODIMP CLTDMLoader::ScanDirectory(
	REFCLSID rguidClass,	// Class id identifies which class of objects this pertains to.
	WCHAR *pszFileExtension,// File extension for type of file to look for. 
							// For example, L"sty" for style files. L"*" will look in all
							// files. L"" or NULL will look for files without an
							// extension.
	WCHAR *pszCacheFileName	// Optional storage file to store and retrieve
							// cached file information. This file is created by 
							// the first call to <om IDirectMusicLoader::ScanDirectory>
							// and used by subsequant calls. NULL if cache file
							// not desired.
)

{
    return E_NOTIMPL;
}


STDMETHODIMP CLTDMLoader::CacheObject(
	IDirectMusicObject * pObject)	// Object to cache.

{
    return E_NOTIMPL;
}


STDMETHODIMP CLTDMLoader::ReleaseObject(
	IDirectMusicObject * pObject)	// Object to release.

{
    return E_NOTIMPL;
}

STDMETHODIMP CLTDMLoader::ClearCache(
	REFCLSID rguidClass)	// Class id identifies which class of objects to clear.
					        // Optionally, GUID_DirectMusicAllTypes specifies all types. 

{
    return E_NOTIMPL;
}

STDMETHODIMP CLTDMLoader::EnableCache(
	REFCLSID rguidClass,	// Class id identifies which class of objects to cache.
					        // Optionally, GUID_DirectMusicAllTypes specifies all types. 
	BOOL fEnable)	        // TRUE to enable caching, FALSE to clear and disable.
{
    return E_NOTIMPL;
}

STDMETHODIMP CLTDMLoader::EnumObject(
	REFCLSID rguidClass,	// Class ID for class of objects to view. 
	unsigned long dwIndex,			// Index into list. Typically, starts with 0 and increments.
	LPDMUS_OBJECTDESC pDESC)// DMUS_OBJECTDESC structure to be filled with data about object.
									   
{
    return E_NOTIMPL;
}


// Functions to support new IDirectMusic8 calls

STDMETHODIMP_(void) CLTDMLoader::CollectGarbage()
{
}

STDMETHODIMP CLTDMLoader::ReleaseObjectByUnknown(IUnknown *pObject)
{
	return E_NOTIMPL;
}

STDMETHODIMP CLTDMLoader::LoadObjectFromFile(REFGUID rguidClassID, 
                                REFIID iidInterfaceID, 
                                WCHAR *pwzFilePath, 
                                void ** ppObject)
{
	return E_NOTIMPL;
}
