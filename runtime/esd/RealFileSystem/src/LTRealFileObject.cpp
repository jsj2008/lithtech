/****************************************************************************
 *
 *  $Id: fileobj1.cpp,v 1.19 1999/07/29 22:02:43 paulm Exp $
 *
 *  Copyright (C) 1995,1996,1997 RealNetworks, Inc.
 *  All rights reserved.
 *
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary information of RealNetworks, Inc.,
 *  and is licensed subject to restrictions on use and distribution.
 *
 *  fileobj1.cpp
 *
 *  Example File Object for the RealMedia Architecture (RMA).
 */

/****************************************************************************
 *
 *  This is an example of a File Object which is used in conjunction with the
 *  File System plug-in to provide low level access to files. The File System
 *  plug-in passes this object to the RMA core, which eventually hands it off
 *  to the appropriate File Format plug-in. This File Format plug-in is what
 *  actually calls the file access methods defined by this object. Thus, this
 *  File Object acts as an abstract interface which File Format objects use
 *  to access the contents of a file.
 *
 *  In order the File Object to achieve the proper functionality required by
 *  a File System plug-in, several interfaces must be implemented. The
 *  IRMAFileObject interface handles all of the file access routines and is
 *  the core of this example. In order to obtain the requested URL, the
 *  IRMARequestHandler is also needed. To determine if the file actually
 *  exists, the IRMAFileExists interface is also implemented.
 *
 *
 *  Example: fileobj1.cpp
 *
 *  This example makes use of the standard C file access routines such as
 *  fread, fwrite, and fseek to handle file access with a file on a local
 *  file system such as your hard drive.
 */



/****************************************************************************
 * Includes
 */
#include <stdio.h>     /* FILE */
#include <string.h>    /* strcpy, etc. */
#if defined (_WINDOWS ) && defined (_WIN32)
#include <direct.h>    /* mkdir, etc. */
#include <io.h>        /* struct _finddata_t, etc. */
#endif

#include "pntypes.h"
#include "pncom.h"     /* IUnknown */
#include "rmacomm.h"   /* IRMACommonClassFactory */
#include "rmapckts.h"  /* IRMAValues, IRMABuffers */

#include "LTRealFileSystem.h"  /* LITH_FILE_SYS_PROTOCOL */
#include "LTRealFileObject.h"  /* LTRealFileObject */

#if defined (_MACINTOSH)
#include <stat.h>      /* struct stat */
#include <unix.h>      /* fileno */
#endif

// LTRealFileObject Class Methods

/****************************************************************************
 *  LTRealFileObject::LTRealFileObject               ref:  fileobj1.h
 *
 *  Constructor
 */
LTRealFileObject::LTRealFileObject
	(IRMACommonClassFactory* pClassFactory, char* pBasePath)
	: m_RefCount       (0),
	  m_pClassFactory  (pClassFactory),
	  m_pFileResponse  (NULL),
	  m_pDirResponse   (NULL),
	  m_pFilename      (NULL),
	  m_pRequest       (NULL),
#if defined (_WINDOWS ) && defined (_WIN32)
	  m_lFileHandle    (0),
#endif
	  m_FileAccessMode (0)
{
	// Signify that we need to keep a reference to this object
	if (m_pClassFactory != NULL)
	{
		m_pClassFactory->AddRef();
	}
	
	strcpy(m_BasePath, pBasePath);

	m_FPInit = NULL;
	m_FPGetFilename = NULL;
	m_FPRead = NULL;
	m_FPWrite = NULL;
	m_FPSeek = NULL;
	m_FPAdvise = NULL;
	m_FPClose = NULL;
	m_FPSetRequest = NULL;
	m_FPGetRequest = NULL;
	m_FPStat = NULL;
}


/****************************************************************************
 *  LTRealFileObject::~LTRealFileObject              ref:  fileobj1.h
 *
 *  Destructor. It is essential to call the Close() routine before destroying
 *  this object.
 */
LTRealFileObject::~LTRealFileObject(void)
{
	Close();
}

// IRMAFileObject Interface Methods

/****************************************************************************
 *  IRMAFileObject::Init                                     ref:  rmafiles.h
 *
 *  This routine associates this File Object with a File Response object
 *  which is notified when file operations (read, write, seek, etc.) are
 *  complete. This method also checks the validity of the file by actually
 *  opening it.
 */
STDMETHODIMP
LTRealFileObject::Init
(
	UINT32            fileAccessMode,
	IRMAFileResponse* pFileResponse
)
{
	if (m_FPInit)
	{
		PN_RESULT result = m_FPInit(fileAccessMode, pFileResponse, m_pClassFactory);
		return result;
	}

	return PNR_FAIL;
}


/****************************************************************************
 *  IRMAFileObject::GetFilename                              ref:  rmafiles.h
 *
 *  This routine returns the name of the requested file (without any path
 *  information). This method may be called by the File Format plug-in if the
 *  short name of the file is required.
 */
STDMETHODIMP
LTRealFileObject::GetFilename(REF(const char*) pFileName)
{
	if (m_FPGetFilename)
	{
		PN_RESULT result = m_FPGetFilename(pFileName);
		return result;
	}

	return PNR_FAIL;
}


/****************************************************************************
 *  IRMAFileObject::Read                                     ref:  rmafiles.h
 *
 *  This routine reads a block of data of the specified length from the file.
 *  When reading has completed, the caller is asynchronously notified via the
 *  File Response object associated with this File Object. This method is 
 *  called by the File Format plug-in when it needs to read from the file.
 */
STDMETHODIMP
LTRealFileObject::Read(UINT32 byteCount)
{
	if (m_FPRead)
	{
		PN_RESULT result = m_FPRead(byteCount);
		return result;
	}

	return PNR_FAIL;
}


/****************************************************************************
 *  IRMAFileObject::Write                                    ref:  rmafiles.h
 *
 *  This routine writes a block of data to the file. When writing has
 *  completed, the caller is asynchronously notified via the File Response
 *  object associated with this File Object. This method called by the File
 *  Format plug-in when it needs to write to the file.
 */
STDMETHODIMP
LTRealFileObject::Write(IRMABuffer* pDataToWrite)
{
	if (m_FPWrite)
	{
		PN_RESULT result = m_FPWrite(pDataToWrite);
		return result;
	}

	return PNR_FAIL;
}


/****************************************************************************
 *  IRMAFileObject::Seek                                    ref:  rmafiles.h
 *
 *  This routine moves to a given position in the file. The move can be
 *  either from the beginning of the file (absolute), or relative to the
 *  current file position. When seeking has completed, the caller is
 *  asynchronously notified via the File Response object associated with this
 *  File Object. This method called by the File Format plug-in when it needs
 *  to seek to a location within the file.
 */
STDMETHODIMP
LTRealFileObject::Seek
(
	UINT32 offset,
	BOOL   bIsRelative
)
{
	if (m_FPSeek)
	{
		PN_RESULT result = m_FPSeek(offset, bIsRelative);
		return result;
	}

	return PNR_FAIL;
}


/****************************************************************************
 *  IRMAFileObject::Advise                                   ref:  rmafiles.h
 *
 *  This routine is passed information about the intended usage of this
 *  object. The useage will indicate whether sequential or random requests
 *  for information will be made. This may be useful, for example, in
 *  developing a caching scheme.
 */
STDMETHODIMP
LTRealFileObject::Advise(UINT32 /* useage */)
{
	if (m_FPAdvise)
		m_FPAdvise(0);

	return PNR_UNEXPECTED;
}


/****************************************************************************
 *  IRMAFileObject::Close                                    ref:  rmafiles.h
 *
 *  This routine closes the file resource and releases all resources
 *  associated with the object. This routine is crucial and must be called
 *  before the File Object is destroyed.
 */
STDMETHODIMP
LTRealFileObject::Close(void)
{
	if (m_FPClose)
		m_FPClose();

	if (m_pClassFactory != NULL)
	{
		m_pClassFactory->Release();
		m_pClassFactory = NULL;
	}

	if (m_pRequest != NULL)
	{
		m_pRequest->Release();
		m_pRequest = NULL;
	}

	if (m_pFilename != NULL)
	{
		delete[] m_pFilename;
		m_pFilename = 0;
	}

#if defined (_WINDOWS ) && defined (_WIN32)
	if (m_lFileHandle)
	{
		_findclose(m_lFileHandle);
		m_lFileHandle = 0;
	}
#endif

	/*
	 * Store this in temp so that if calling CloseDone
	 * causes our descructor to get called we will
	 * have pCallCloseDone on the stack to safely release.
	 */
	IRMAFileResponse* pCallCloseDone = m_pFileResponse;
	if (m_pFileResponse != NULL)
	{
	    m_pFileResponse = NULL;
	    pCallCloseDone->CloseDone(PNR_OK);
	    pCallCloseDone->Release();
	}

	return PNR_OK;
}


// IRMARequestHandler Interface Methods

/****************************************************************************
 *  IRMARequestHandler::SetRequest                           ref:  rmafiles.h
 *
 *  This routine associates this File Object with the file Request object
 *  passed to it from the RMA core. This Request object is primarily used to
 *  obtain the requested URL. This method is called just after the File
 *  Object is created.
 */
STDMETHODIMP
LTRealFileObject::SetRequest(IRMARequest* pRequest)
{
	if (m_FPSetRequest)
	{
		PN_RESULT result = m_FPSetRequest(pRequest);
		return result;
	}

	return PNR_FAIL;
}


/****************************************************************************
 *  IRMARequestHandler::GetRequest                           ref:  rmafiles.h
 *
 *  This routine retrieves the Request object associated with this File
 *  Object. It is called just after the SetRequest() method.
 */
STDMETHODIMP
LTRealFileObject::GetRequest(REF(IRMARequest*) pRequest)
{
	if (m_FPGetRequest)
	{
		PN_RESULT result = m_FPGetRequest(pRequest);
		return result;
	}

	return PNR_FAIL;
}


/****************************************************************************
 *  IRMAFileExists::DoesExist                                ref:  rmafiles.h
 *
 *  This routine determines if the given file exists, and notifies the File
 *  Response object. It is called by the RMA server after the File Object has
 *  been created to determine if the requested file does actually exist. If
 *  it does the File Object is handed off to the File Format object.
 */
STDMETHODIMP
LTRealFileObject::DoesExist
(
	const char*             pFilePathRMA,
	IRMAFileExistsResponse* pFileResponse
)
{
/*	BOOL  bDoesExist = FALSE;
	char* pFilePathPlatform   = NULL;

	// Convert RMA file path to platform specific file path
	PN_RESULT result = MyConvertToPlatformPath(pFilePathPlatform,pFilePathRMA);

	// Determine if the file can be opened
	if (result == PNR_OK)
	{
		struct stat statbuf;
		if (stat(pFilePathPlatform, &statbuf) == 0)
		{
			bDoesExist = TRUE;
		}
	}

	// Notify the caller if the file exists
	pFileResponse->DoesExistDone(bDoesExist);

	if (pFilePathPlatform != NULL)
	{
		delete[] pFilePathPlatform;
	}*/

	return PNR_OK;
}


// IUnknown COM Interface Methods

/****************************************************************************
 *  IUnknown::AddRef                                            ref:  pncom.h
 *
 *  This routine increases the object reference count in a thread safe
 *  manner. The reference count is used to manage the lifetime of an object.
 *  This method must be explicitly called by the user whenever a new
 *  reference to an object is used.
 */
STDMETHODIMP_(UINT32)
LTRealFileObject::AddRef(void)
{
	return InterlockedIncrement(&m_RefCount);
}


/****************************************************************************
 *  IUnknown::Release                                           ref:  pncom.h
 *
 *  This routine decreases the object reference count in a thread safe
 *  manner, and deletes the object if no more references to it exist. It must
 *  be called explicitly by the user whenever an object is no longer needed.
 */
STDMETHODIMP_(UINT32)
LTRealFileObject::Release(void)
{
	if (InterlockedDecrement(&m_RefCount) > 0)
	{
		return m_RefCount;
	}

	delete this;
	return 0;
}


/****************************************************************************
 *  IUnknown::QueryInterface                                    ref:  pncom.h
 *
 *  This routine indicates which interfaces this object supports. If a given
 *  interface is supported, the object's reference count is incremented, and
 *  a reference to that interface is returned. Otherwise a NULL object and
 *  error code are returned. This method is called by other objects to
 *  discover the functionality of this object.
 */
STDMETHODIMP
LTRealFileObject::QueryInterface
(
	REFIID interfaceID,
	void** ppInterfaceObj
)
{
	// By definition all COM objects support the IUnknown interface
	if (IsEqualIID(interfaceID, IID_IUnknown))
	{
		AddRef();
		*ppInterfaceObj = (IUnknown*)(IRMAFileObject*)this;
		return PNR_OK;
	}

	// IRMAFileObject interface is supported
	else if (IsEqualIID(interfaceID, IID_IRMAFileObject))
	{
		AddRef();
		*ppInterfaceObj = (IRMAFileObject*)this;
		return PNR_OK;
	}

	// IRMADirHandler interface is supported
	else if(IsEqualIID(interfaceID, IID_IRMADirHandler))
	{
		AddRef();
		*ppInterfaceObj = (IRMADirHandler*)this;
		return PNR_OK;
	}
	// IRMARequestHandler interface is supported
	else if (IsEqualIID(interfaceID, IID_IRMARequestHandler))
	{
		AddRef();
		*ppInterfaceObj = (IRMARequestHandler*)this;
		return PNR_OK;
	}

	// IRMAFileExists interface is supported
	else if (IsEqualIID(interfaceID, IID_IRMAFileExists))
	{
		AddRef();
		*ppInterfaceObj = (IRMAFileExists*)this;
		return PNR_OK;
	}
	else if (IsEqualIID(interfaceID, IID_IRMAFileStat))
	{
		AddRef();
		*ppInterfaceObj = (IRMAFileStat*)this;
		return PNR_OK;
	}
	else if (IsEqualIID(interfaceID, IID_IRMAGetFileFromSamePool))
	{
	    AddRef();
	    *ppInterfaceObj = (IRMAGetFileFromSamePool*)this;
	    return PNR_OK;
	}

	// No other interfaces are supported
	*ppInterfaceObj = NULL;
	return PNR_NOINTERFACE;
}

/************************************************************************
 * Method:
 *	IRMAFileObject::Stat
 * Purpose:
 *	Collects information about the file that is returned to the
 *	caller in an IRMAStat object
 */
STDMETHODIMP 
LTRealFileObject::Stat(IRMAFileStatResponse* pFileStatResponse)
{
	if (m_FPStat)
	{
		PN_RESULT result = m_FPStat(pFileStatResponse);
		return result;
	}

	return PNR_FAIL;
}


STDMETHODIMP
LTRealFileObject::InitDirHandler (IRMADirHandlerResponse* pDirResponse)
{
    m_pDirResponse = pDirResponse;
    m_pDirResponse->AddRef();
    m_pDirResponse->InitDirHandlerDone(PNR_OK);

    return PNR_OK;
}

STDMETHODIMP
LTRealFileObject::CloseDirHandler()
{
    m_pDirResponse->CloseDirHandlerDone(PNR_OK);
    m_pDirResponse->Release();
    m_pDirResponse = NULL;

    return PNR_OK;
}

STDMETHODIMP
LTRealFileObject::MakeDir()
{
#if defined (_WINDOWS ) || defined (_WIN32)
    if(mkdir(m_pFilename) < 0)
        return PNR_FAIL;
#elif defined UNIX
    if(mkdir(m_pFilename, 0x644) < 0)
        return PNR_FAIL;
#else
    //XXXGH...don't know how to do this on Mac
    return PNR_FAIL;
#endif

    m_pDirResponse->MakeDirDone(PNR_OK);

    return PNR_OK;
}

STDMETHODIMP
LTRealFileObject::ReadDir()
{
    char pDirname[1024];

#if defined (_WINDOWS ) && defined (_WIN32)
    if (!m_lFileHandle)
    {
	strcpy(pDirname, m_pFilename);
	strcat(pDirname, "\\*");

	m_lFileHandle = _findfirst(pDirname, &m_FileInfo);

	if (m_lFileHandle < 0)
	{
	    m_lFileHandle = 0;
	    return PNR_FAILED;
	}
    }
    else
    {
	if (_findnext(m_lFileHandle, &m_FileInfo) < 0)
	{
	    _findclose(m_lFileHandle);
	    m_lFileHandle = 0;
	    m_pDirResponse->ReadDirDone(PNR_FILE_NOT_FOUND, 0);
	    return PNR_OK;
	}
    }

    strcpy(pDirname, m_FileInfo.name);
#else
    return PNR_FAIL;
#endif

    PN_RESULT result;

    IRMABuffer* pBuffer = 0;

    result = m_pClassFactory->CreateInstance(CLSID_IRMABuffer,
                                                   (void**)&pBuffer);

    if (PNR_OK != result)
    {
	return result;
    }

    pBuffer->Set((Byte*)pDirname, strlen(pDirname)+1);
    m_pDirResponse->ReadDirDone(PNR_OK, pBuffer);
    pBuffer->Release();

    return PNR_OK;
}

/************************************************************************
 *	Method:
 *	    IRMAFileObject::GetFileObjectFromPool
 *	Purpose:
 *      To get another FileObject from the same pool. 
 */
STDMETHODIMP 
LTRealFileObject::GetFileObjectFromPool (
    IRMAGetFileFromSamePoolResponse* response
)
{
    PN_RESULT lReturnVal = PNR_FAILED;
    LTRealFileObject* pFileObject = 0;
    IUnknown* pUnknown = 0;

    // Chop off the current path to create a base path.
    strcpy(m_BasePath,m_pFilename);
    char* pLastSeparator = strrchr(m_BasePath,OS_PATH_SEPARATOR_CHAR);
    *pLastSeparator = '\0';

    LT_MEM_TRACK_ALLOC(pFileObject = new LTRealFileObject(m_pClassFactory,m_BasePath),LT_MEM_TYPE_MISC);

    if (!pFileObject)
    {
	return PNR_OUTOFMEMORY;
    }

    lReturnVal = pFileObject->QueryInterface(IID_IUnknown,
					     (void**)&pUnknown);
    
    response->FileObjectReady(lReturnVal == PNR_OK ? 
			      PNR_OK : PNR_FAILED,
			      pUnknown);
    if(pUnknown)
    {
	pUnknown->Release();
	pUnknown = NULL;
    }

    return lReturnVal;
}
