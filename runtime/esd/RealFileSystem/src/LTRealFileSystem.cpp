/****************************************************************************
 *
 *  $Id: filesys1.cpp,v 1.17 1998/05/21 19:22:29 brad Exp $
 *
 *  Copyright (C) 1995,1996,1997 RealNetworks, Inc.
 *  All rights reserved.
 *
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary information of RealNetworks, Inc.,
 *  and is licensed subject to restrictions on use and distribution.
 *
 *  filesys1.cpp
 *
 *  Example File System plug-in for the RealMedia Architecture (RMA).
 */

/****************************************************************************
 *
 *  This is an example of a File System plug-in. The purpose of a File System
 *  plug-in is to handle low level file I/O operations such as reading and
 *  writing. More importantly this type of plug-in acts as an abstract
 *  interface which allows the *File Format* plug-in to access a file using
 *  the methods defined by this File System plug-in. For example, when a
 *  File Format plug-in needs to read the contents of a file, it calls the
 *  Read() method by way of this "virtual" File Object created by the File
 *  System plug-in. This File System object handles all the low level read
 *  operations and returns the data requested to the File Format plug-in. 
 *
 *  Different File System plug-ins handle different types of "files". A
 *  file could be a traditional data file stored on your hard drive, or it
 *  may be a different type of stored data altogether, such as a database
 *  record for example. In any case, the RMA core decides which File System
 *  plug-in to use according to the protocol of the requested URL. If for
 *  example the URL "file://myfile.txt" is requested, its protocol is "file",
 *  and the File System plug-in associated with this protocol would be used
 *  to access the "myfile.txt" file.
 *  
 *  To summarize, the general sequence of events is as follows. An URL is
 *  requested to be opened. The appropriate File Format plug-in is loaded
 *  depending on the file's MIME type or extension. The File Format plug-in
 *  reads the contents of the file via a virtual File System object. The
 *  appropriate File System plug-in has already been loaded according to the
 *  type of protocol in the requested URL. The methods defined in the File
 *  System plug-in handle all the low level file operations.
 *
 *  Your File System plug-in must implement certain interfaces in order to 
 *  achieve this functionality. The plug-in must implement the
 *  IRMAFileSystemObject interface to handle the basic File System
 *  functionality. The heart of this interface is the CreateFile() method
 *  which is responsible for returning a File Object which does most of the
 *  work. Refer to the documentation in "fileobj1.cpp" for a detailed
 *  description of that object.
 *
 *
 *  Example: filesys1.cpp
 *
 *  This is an example of a File System plug-in which handles file operations
 *  for files stored on your local file system (hard disk, floppy, etc.).
 *  This plug-in simply uses the standard C file access routines (fread, 
 *  fwrite, etc.) to implement its functionality.
 *
 *
 *  The RMA core calls the various interface routines at well defined times.
 *  The sequence of the major events is as follows:
 *
 *  Upon RMA core startup:
 *  - RMACreateInstance()  creates new instance of LTRealFileSystem class
 *  - GetPlugInInfo()      returns descriptive information about the plug-in
 *  - GetFileSystemInfo()  returns crucial info linking plug-in and protocol
 *
 *  When an URL is requested:
 *  - RMACreateInstance()  creates new instance of LTRealFileSystem class
 *  - InitPlugIn()         initializes plug-in
 *  - InitFileSystem()     performs any additional initialization
 *  - CreateFile()         creates the File Object (see fileobj1.cpp)
 *    ...SetRequest()      stores an object used to obtain requested URL
 *    ...GetRequest()      retrieves above object
 *    ...Init()            initializes File Object 
 *    ...Seek()            called when File Format seeks within the file
 *    ...Read()            called when File Format reads the file
 *    ...Write()           called when File Format writes to the file
 *  
 */


/****************************************************************************
 * Defines
 */
#define   INITGUID     /* Interface ID's */


/****************************************************************************
 * Includes
 */
#include <string.h>    /* strcpy */

#include "pntypes.h" 
#include "pncom.h"     /* IUnknown */
#include "rmacomm.h"   /* IRMACommonClassFactory */
#include "rmapckts.h"  /* IRMAValues */

#include "LTRealFileSystem.h"		/* LTRealFileSystem */
#include "LTRealFileObject.h"		/* LTRealFileObject */

/****************************************************************************
 *  RMACreateInstance                                        ref:  rmaplugn.h
 *
 *  This routine creates a new instance of the LTRealFileSystem class.
 *  It is called when the RMA core application is launched, and whenever
 *  an URL with a protocol associated with this plug-in is opened.
 */
STDAPI
RMACreateInstance(IUnknown** ppExFileSystemObj)
{
	LT_MEM_TRACK_ALLOC(*ppExFileSystemObj = (IUnknown*)(IRMAPlugin*)new LTRealFileSystem(),LT_MEM_TYPE_MISC);
	if (*ppExFileSystemObj != NULL)
	{
		(*ppExFileSystemObj)->AddRef();
		return PNR_OK;
	}

	return PNR_OUTOFMEMORY;
}


//-----------------------------------------------------------------------------
// File system hooks
//-----------------------------------------------------------------------------
static FPInit g_FPInit = NULL;
static FPGetFilename g_FPGetFilename = NULL;
static FPRead g_FPRead = NULL;
static FPWrite g_FPWrite = NULL;
static FPSeek g_FPSeek = NULL;
static FPAdvise g_FPAdvise = NULL;
static FPClose g_FPClose = NULL;
static FPSetRequest g_FPSetRequest = NULL;
static FPGetRequest g_FPGetRequest = NULL;
static FPStat g_FPStat = NULL;

void BindFunctions(FPBindBlock* pBlock)
{
	if (pBlock)
	{
		g_FPInit = pBlock->m_FPInit;
		g_FPGetFilename = pBlock->m_FPGetFilename;
		g_FPRead = pBlock->m_FPRead;
		g_FPWrite = pBlock->m_FPWrite;
		g_FPSeek = pBlock->m_FPSeek;
		g_FPAdvise = pBlock->m_FPAdvise;
		g_FPClose = pBlock->m_FPClose;
		g_FPSetRequest = pBlock->m_FPSetRequest;
		g_FPGetRequest = pBlock->m_FPGetRequest;
		g_FPStat = pBlock->m_FPStat;
	}
}
//-----------------------------------------------------------------------------



// LTRealFileSystem Class Methods

/****************************************************************************
 *  LTRealFileSystem static variables                    ref:  filesys1.h
 *
 *  These variables are passed to the RMA core to provide information about
 *  this plug-in. They are required to be static in order to remain valid
 *  for the lifetime of the plug-in.
 */
const char* LTRealFileSystem::zm_pDescription = LITH_DESCRIPTION;
const char* LTRealFileSystem::zm_pCopyright   = LITH_COPYRIGHT;
const char* LTRealFileSystem::zm_pMoreInfoURL = LITH_MORE_INFO_URL;
const char* LTRealFileSystem::zm_pShortName   = LITH_FILE_SYS_NAME;
const char* LTRealFileSystem::zm_pProtocol    = LITH_FILE_SYS_PROTOCOL;


/****************************************************************************
 *  LTRealFileSystem::LTRealFileSystem               ref:  filesys1.h
 *
 *  Constructor
 */
LTRealFileSystem::LTRealFileSystem(void)
	: m_RefCount      (0),
	  m_pClassFactory (NULL)
{
	m_BasePath[ 0 ] = '\0';
}


/****************************************************************************
 *  LTRealFileSystem::~LTRealFileSystem              ref:  filesys1.h
 *
 *  Destructor. Be sure to release all outstanding references to objects.
 */
LTRealFileSystem::~LTRealFileSystem(void)
{
	if (m_pClassFactory != NULL)
	{
		m_pClassFactory->Release();
		m_pClassFactory = NULL;
	}
}


// IRMAFileSystemObject Interface Methods

/****************************************************************************
 *  IRMAFileSystemObject::GetFileSystemInfo                  ref:  rmafiles.h
 *
 *  This routine returns crucial information required to associate this
 *  plug-in with a given protocol. This information tells the core which
 *  File System plug-in to use for a particular protocol. For example, in the
 *  URL: "file://myfile.txt", the protocol would be "file". This routine is
 *  called when the RMA core application is launched.
 */
STDMETHODIMP
LTRealFileSystem::GetFileSystemInfo
(
	REF(const char*) pShortName,
	REF(const char*) pProtocol
)
{
	pShortName = zm_pShortName;
	pProtocol  = zm_pProtocol;

	return PNR_OK;
}


/****************************************************************************
 *  IRMAFileSystemObject::InitFileSystem                     ref:  rmafiles.h
 *
 *  This routine performs any additional initialization steps required for
 *  the file system.  It is called prior to the CreatFile() request. Any
 *  options provided usually refer to mounting options related to the server,
 *  such as base path or authentication preferences. 
 */
STDMETHODIMP 
LTRealFileSystem::InitFileSystem(IRMAValues*  options )
{
	// Retrieve the platform's base path, if specified
	if (options != NULL )
	{
		IRMABuffer* pPathBuffer = NULL;
		if (options->GetPropertyBuffer("BasePath", pPathBuffer) == PNR_OK)
		{
			if (pPathBuffer->GetBuffer() != NULL)
			{
				strcpy(m_BasePath, (char*)pPathBuffer->GetBuffer());
			}
			pPathBuffer->Release();
		}
	}
	
	return PNR_OK;
}


/****************************************************************************
 *  IRMAFileSystemObject::CreateFile                        ref:  rmafiles.h
 *
 *  This routine creates a new File Object which handles all of the file I/O
 *  functionality of this class. This File Object is eventually handed off
 *  to a File Format plug-in which handles file I/O through this File Object.
 *  This method is called called when an URL with a protocol associated with
 *  this plug-in is opened.
 */
STDMETHODIMP
LTRealFileSystem::CreateFile(IUnknown** ppFileObject)
{
	LTRealFileObject* pFileObj;

	// Create a new File Object which implements the file I/O methods
	LT_MEM_TRACK_ALLOC(pFileObj = new LTRealFileObject(m_pClassFactory, m_BasePath),LT_MEM_TYPE_MISC);
	if (pFileObj != NULL)
	{
		pFileObj->m_FPInit = g_FPInit;
		pFileObj->m_FPGetFilename = g_FPGetFilename;
		pFileObj->m_FPRead = g_FPRead;
		pFileObj->m_FPWrite = g_FPWrite;
		pFileObj->m_FPSeek = g_FPSeek;
		pFileObj->m_FPAdvise = g_FPAdvise;
		pFileObj->m_FPClose = g_FPClose;
		pFileObj->m_FPSetRequest = g_FPSetRequest;
		pFileObj->m_FPGetRequest = g_FPGetRequest;
		pFileObj->m_FPStat = g_FPStat;

		/*
		 * Pass the File Object off to the RMA core, and eventually to the
		 * associated File Format plug-in.
		 */
		pFileObj->QueryInterface(IID_IUnknown, (void**)ppFileObject);
		if (pFileObj != NULL)
		{
			return PNR_OK;
		}
		return PNR_UNEXPECTED;
		/*
		 * Note that the RMA core obtains ownership of this File Object and
		 * is therefore responsible for releasing it.
		 */
	}

	return PNR_OUTOFMEMORY;
}


/****************************************************************************
 *  IRMAFileSystemObject::CreateDir                          ref:  rmafiles.h
 *
 *  This routine is analagous to CreatFile, except directories instead of
 *  files are of concern. It is not implemented in this example.
 */
STDMETHODIMP
LTRealFileSystem::CreateDir(IUnknown** /* ppDirectoryObject */)
{
	return PNR_NOTIMPL;
}


// IRMAPlugin Interface Methods

/****************************************************************************
 *  IRMAPlugin::GetPluginInfo                                ref:  rmaplugn.h
 *
 *  This routine returns descriptive information about the plug-in, most
 *  of which is used in the About box of the user interface. It is called
 *  when the RMA core application is launched.
 */
STDMETHODIMP
LTRealFileSystem::GetPluginInfo
(
	REF(BOOL)        bLoadMultiple,
	REF(const char*) pDescription,
	REF(const char*) pCopyright,
	REF(const char*) pMoreInfoURL,
	REF(UINT32)      versionNumber
)
{
	bLoadMultiple = TRUE;
	pDescription  = zm_pDescription;
	pCopyright    = zm_pCopyright;
	pMoreInfoURL  = zm_pMoreInfoURL;
	versionNumber = LITH_PLUGIN_VERSION;

	return PNR_OK;
}


/****************************************************************************
 *  IRMAPlugin::InitPlugin                                   ref:  rmaplugn.h
 *
 *  This routine performs initialization steps such as determining if
 *  required interfaces are available. It is called when the RMA core 
 *  application is launched, and whenever an URL with a protocol associated
 *  with this plug-in is opened.
 */
STDMETHODIMP
LTRealFileSystem::InitPlugin(IUnknown* pRMACore)
{
	/*
	 * Store a reference to the IRMACommonClassFactory interface which is
	 * used to create commonly used RMA objects such as IRMAPacket, 
	 * IRMAValues, and IRMABuffers.
	 */
	if (pRMACore->QueryInterface(IID_IRMACommonClassFactory, 
                                           (void**)&m_pClassFactory) != PNR_OK)
	{
		return PNR_NOINTERFACE;
	}

	/* 
	 * Note that QueryInterface() takes care of adding a reference to the
	 * interface for us. You are however responsible for releasing the
	 * reference to the interface when you are done using it by calling
	 * the Release() routine.
	 */

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
LTRealFileSystem::AddRef(void)
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
LTRealFileSystem::Release(void)
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
LTRealFileSystem::QueryInterface
(
	REFIID interfaceID,
	void** ppInterfaceObj
)
{
	// By definition all COM objects support the IUnknown interface
	if (IsEqualIID(interfaceID, IID_IUnknown))
	{
		AddRef();
		*ppInterfaceObj = (IUnknown*)(IRMAPlugin*)this;
		return PNR_OK;
	}

	// IRMAPlugin interface is supported
	else if (IsEqualIID(interfaceID, IID_IRMAPlugin))
	{
		AddRef();
		*ppInterfaceObj = (IRMAPlugin*)this;
		return PNR_OK;
	}

	// IRMAFileSystemObject interface is supported
	else if (IsEqualIID(interfaceID, IID_IRMAFileSystemObject))
	{
		AddRef();
		*ppInterfaceObj = (IRMAFileSystemObject*)this;
		return PNR_OK;
	}

	// No other interfaces are supported
	*ppInterfaceObj = NULL;
	return PNR_NOINTERFACE;
}
