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


//#include "bdefs.h"

#include "dmusici.h"

#include "lith.h"
#include "ltdirectmusicloader.h"


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
	CLoadDirByClassItem* pNew = new CLoadDirByClassItem;
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
	CFileStreamItem* pNewItem = new CFileStreamItem;
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

CLTDMFileStream::CLTDMFileStream( CLTDMLoader *pLoader)

{
	m_cRef = 1;         // Start with one reference for caller.

#ifdef USE_DSTREAM
//	m_pDStream = NULL;
//	m_DStreamFileRef.m_pFilename = NULL;
	m_pOpenQueueItem = NULL;
#endif

#ifdef USE_REZMGR
	m_pRezItem = NULL;
#endif

#ifdef USE_FILE
	m_pFile = NULL;     // No file yet.
#endif

	m_pLoader = pLoader; // Link to loader, so loader can be found from stream.
	if (pLoader)
	{
		pLoader->AddRefP(); // Addref the private counter to avoid cyclic references.

		// add to list of file streams
		pLoader->GetFileStreamList()->AddItem(this);
	}
}

CLTDMFileStream::~CLTDMFileStream()

{
	if (m_pLoader)
	{
		// remove from list of file streams
		m_pLoader->GetFileStreamList()->DeleteItem(this);

		m_pLoader->ReleaseP();
	}
	Close();
}


HRESULT CLTDMFileStream::Open(WCHAR * lpFileName,unsigned long dwDesiredAccess)
{
#ifdef USE_DSTREAM
	Close();

	// make sure file name is not null
	if (lpFileName == NULL)
	{
		return E_FAIL;
	}

	// convert file name to regular characters
	char sFileName[MAX_PATH];
	wcstombs( sFileName, lpFileName, MAX_PATH );

	// create an item in the OpenQueueMgr
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

	// we succeed
	return S_OK;
#endif

#ifdef USE_REZMGR
	// convert the file name to regular characters from wide
	char sFileName[MAX_PATH];
	wcstombs( sFileName, lpFileName, MAX_PATH );

	// if the loader doesn't exist fail
	if (m_pLoader == NULL)
	{
		LTDMConOutWarning("LTDirectMusic loader failed to open file %s because no directmusic loader exists\n", sFileName);
		return E_FAIL;
	}

	m_pLoader->EnterRezMgrCriticalSection();

	// if the rezmgr is not open then fail
	if (!m_pLoader->GetRezMgr()->IsOpen())
	{
		LTDMConOutWarning("LTDirectMusic loader failed to open file %s because rez file was not open\n", sFileName);
		m_pLoader->LeaveRezMgrCriticalSection();
		return E_FAIL;
	}

	// we don't support write access so fail if it was requested
	if( dwDesiredAccess == GENERIC_WRITE )
	{
		LTDMConOutWarning("LTDirectMusic loader failed to open file %s for writing because writing is not supported\n", sFileName);
		m_pLoader->LeaveRezMgrCriticalSection();
		return DMUS_E_LOADER_FAILEDOPEN;
	}

	// get the rez item
	m_pRezItem = m_pLoader->GetRezMgr()->GetRezFromDosPath(sFileName);

	// if the item was not found we fail
	if (m_pRezItem == NULL)
	{
		LTDMConOutWarning("LTDirectMusic loader failed to open file %s\n", sFileName);
		m_pLoader->LeaveRezMgrCriticalSection();
		return DMUS_E_LOADER_FAILEDOPEN;
	}

	// seek to start of item
//	m_pRezItem->Seek(0);
	m_nCurPos = 0;

	LTDMConOutMsg(5, "CLTDMFileStream::Open FileName=%s\n", sFileName);

	// we succeed
	m_pLoader->LeaveRezMgrCriticalSection();
	return S_OK;
#endif

#ifdef USE_FILE
	Close();
    if( dwDesiredAccess == GENERIC_READ )
    {
		if (g_fIsUnicode)
		{
			m_pFile = _wfsopen( lpFileName, L"rb", _SH_DENYWR );
		}
		else
		{
			char path[MAX_PATH];
			wcstombs( path, lpFileName, MAX_PATH );
			m_pFile = _fsopen( path, "rb", _SH_DENYWR );
		}
	}
    else if( dwDesiredAccess == GENERIC_WRITE )
    {
		if (g_fIsUnicode)
		{
			m_pFile = _wfsopen( lpFileName, L"wb", _SH_DENYNO );
		}
		else
		{
			char path[MAX_PATH];
			wcstombs( path, lpFileName, MAX_PATH );
			m_pFile = _fsopen( path, "wb", _SH_DENYNO );
		}
	}
	if (m_pFile == NULL)
	{
		return DMUS_E_LOADER_FAILEDOPEN;
	}
	return S_OK;
#endif
}


HRESULT CLTDMFileStream::Close()

{
#ifdef USE_DSTREAM
	if (m_pOpenQueueItem != NULL)
	{
		m_pOpenQueueItem->Close();
		g_LTDMDStreamOpenQueueMgr.Destroy(m_pOpenQueueItem);
		m_pOpenQueueItem = NULL;
	}
#endif

#ifdef USE_REZMGR
	m_pRezItem = NULL;
#endif

#ifdef USE_FILE
	if (m_pFile)
	{
		fclose(m_pFile);
	}
	m_pFile = NULL;
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
//	dprintf("CLTDMFileStream::GetLoader this=%x m_cRef=%i\n",this,m_cRef);
	if (m_pLoader)
	{
		return m_pLoader->QueryInterface( IID_IDirectMusicLoader8,(void **) ppLoader );
	}
	*ppLoader = NULL;
	return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CLTDMFileStream::AddRef()
{
//	dprintf("CLTDMFileStream::AddRef this=%x m_cRef=%i\n",this,m_cRef+1);
   return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CLTDMFileStream::Release()
{
//	dprintf("CLTDMFileStream::Release this=%x m_cRef=%i\n",this,m_cRef-1);
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
#ifdef USE_DSTREAM

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

	return S_OK;
#endif

#ifdef USE_REZMGR
	// if the loader doesn't exist fail
	if (m_pLoader == NULL) 
	{
		LTDMConOutError("LTDirectMusic loader failed to read from file because no directmusic loader exists\n");
		return E_FAIL;
	}

	m_pLoader->EnterRezMgrCriticalSection();

	// if the rezmgr is not open
	if (!m_pLoader->GetRezMgr()->IsOpen()) 
	{
		LTDMConOutError("LTDirectMusic loader failed to read from file because rez file was not open\n");
		m_pLoader->LeaveRezMgrCriticalSection();
		return E_FAIL;
	}

	// variable to hold the actual number of bytes that were read in when read is called
	size_t nBytesRead;

	// read the data from the rez file
	nBytesRead = m_pRezItem->Read(pv, cb, m_nCurPos);

	// advance seek pos
	m_nCurPos += nBytesRead;

	// return OK if we read all of the requested data
	if(cb == nBytesRead) 
	{
		// set the number of bytes read callback if it is not NULL
		if(pcbRead != NULL)*pcbRead = nBytesRead;

		m_pLoader->LeaveRezMgrCriticalSection();
		return S_OK;
	}

	// if we didn't read all of the data fail
	else
	{
		m_pLoader->LeaveRezMgrCriticalSection();
		return E_FAIL;
	}

	m_pLoader->LeaveRezMgrCriticalSection();
#endif

#ifdef USE_FILE
	size_t dw;
	dw = fread( pv, sizeof(char), cb, m_pFile );
	if( cb == dw )
	{
		if( pcbRead != NULL )
		{
			*pcbRead = cb;
		}
		return S_OK;
	}
	return E_FAIL ;
#endif
}

STDMETHODIMP CLTDMFileStream::Write( const void* pv, ULONG cb, ULONG* pcbWritten )
{
#ifdef USE_DSTREAM
	return E_FAIL;
#endif

#ifdef USE_REZMGR
	return E_FAIL;
#endif

#ifdef USE_FILE
	if( cb == fwrite( pv, sizeof(char), cb, m_pFile ))
	{
		if( pcbWritten != NULL )
		{
			*pcbWritten = cb;
		}
		return S_OK;
	}
    return STG_E_MEDIUMFULL;
#endif
}

STDMETHODIMP CLTDMFileStream::Seek( LARGE_INTEGER dlibMove, unsigned long dwOrigin, ULARGE_INTEGER* plibNewPosition )
{
#ifdef USE_DSTREAM
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
    return S_OK;
#endif

#ifdef USE_REZMGR
	// if the loader doesn't exist fail
	if (m_pLoader == NULL) 
	{
		LTDMConOutError("LTDirectMusic loader failed to seek from file because no directmusic loader exists\n");
		return E_FAIL;
	}

	m_pLoader->EnterRezMgrCriticalSection();

	// if the rezmgr is not open
	if (!m_pLoader->GetRezMgr()->IsOpen()) 
	{
		LTDMConOutError("LTDirectMusic loader failed to seek from file because rez file was not open\n");
		m_pLoader->LeaveRezMgrCriticalSection();
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
			nCurPos.LowPart = m_nCurPos;;
			nCurPos.HighPart = 0;
			
			LARGE_INTEGER nNewPos;
			nNewPos.QuadPart = nCurPos.QuadPart + dlibMove.QuadPart;

			nSeekPos = nNewPos.LowPart;
		}
		else
		{
			if (dwOrigin == SEEK_END)
			{
				LARGE_INTEGER nSize;
				nSize.LowPart = m_pRezItem->GetSize();
				nSize.HighPart = 0;

				LARGE_INTEGER nNewPos;
				nNewPos.QuadPart = nSize.QuadPart + dlibMove.QuadPart;

				nSeekPos = nNewPos.LowPart;
			}
			else
			{
				// unknown origin we must fail
				LTDMConOutError("LTDirectMusic failed to seek in file %s invalid seek origin\n",m_pRezItem->GetName());
				m_pLoader->LeaveRezMgrCriticalSection();
				return E_FAIL;
			}
		}
	}

	// do the seek
	m_nCurPos = nSeekPos;

	// set the return current position if it was not NULL
	if( plibNewPosition != NULL )
	{
		plibNewPosition->LowPart = m_nCurPos;
		plibNewPosition->HighPart = 0;
	}

	// everything worked so we return successful
	m_pLoader->LeaveRezMgrCriticalSection();
    return S_OK;
#endif

#ifdef USE_FILE
	// fseek can't handle a LARGE_INTEGER seek...
	long lOffset;

	lOffset = dlibMove.LowPart;

	int i = fseek( m_pFile, lOffset, dwOrigin );
	if( i ) 
	{
		return E_FAIL;
	}

	if( plibNewPosition != NULL )
	{
		plibNewPosition->LowPart = ftell( m_pFile );
		if( lOffset < 0 )
		{
			plibNewPosition->HighPart = -1;
		}
		else
		{
			plibNewPosition->HighPart = 0;
		}
	}
    return S_OK;
#endif
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
	HRESULT hr;

	// allocate a new stream object
	CLTDMFileStream* pStream = new CLTDMFileStream( m_pLoader );

	if ( !pStream )
		return E_FAIL;

	// get the IStream interface
	hr = pStream->QueryInterface( IID_IStream, (void**) ppstm );	

	// if everything is OK, set up the seek position
	if ( SUCCEEDED( hr ) )
	{
#ifdef USE_FILE
		pStream->m_pFile = this->m_pFile;
#endif

#ifdef USE_REZMGR
		pStream->m_pRezItem = this->m_pRezItem;
		pStream->m_nCurPos = this->m_nCurPos;
#endif

#ifdef USE_DSTREAM
		pStream->m_pLoader = this->m_pLoader;

		// add a new queue item, identical to the current one
		const char* sFileName = this->m_pOpenQueueItem->GetFileName();
	    CDStreamOpenQueueItem* pOpenQueueItem;
		
		pOpenQueueItem = g_LTDMDStreamOpenQueueMgr.Create(sFileName);
		pStream->m_pOpenQueueItem = pOpenQueueItem;

		if ( !pOpenQueueItem )
			return E_FAIL;

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
#endif
	}

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
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CLTDMMemStream::Release()
{
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
	m_pLoadDirByClassList = new CLoadDirByClassList;
	m_pFileStreamList = new CFileStreamList;
#ifdef USE_DSTREAM
	g_LTDMDStreamOpenQueueMgr.Init(5); // 5 simultaneous ILTStream accesses allowed
#endif
}

CLTDMLoader::~CLTDMLoader()

{
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
	else hr = DMUS_E_LOADER_NOFILENAME;
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
            CLTDMObjectRef * pObject = new CLTDMObjectRef;
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
		CLTDMFileStream *pStream = new CLTDMFileStream ( this );
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
		CLTDMMemStream *pStream = new CLTDMMemStream ( this );
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
		char sFileName[MAX_PATH];
		strcpy(sFileName, sRezName);

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
