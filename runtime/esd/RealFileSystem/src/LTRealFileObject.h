/****************************************************************************
 *
 *  $Id: fileobj1.h,v 1.10 1998/06/13 03:08:23 glennh Exp $
 *
 *  Copyright (C) 1995,1996,1997 RealNetworks, Inc.
 *  All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary information of RealNetworks, Inc.,
 *  and is licensed subject to restrictions on use and distribution.
 *
 *  fileobj1.h
 *
 *  Example file object for the RealMedia Architecture (RMA).
 * 
 */
#ifndef LTRealFileObject_H
#define LTRealFileObject_H


/****************************************************************************
 * Includes
 */
#include "rmafiles.h"  /* IRMAFileObject, IRMARequestHandler, etc. */


/****************************************************************************
 * Constants
 */
#if defined (_WINDOWS) || defined (_WIN32)
  #define OS_PATH_SEPARATOR_CHAR '\\'
  #define OS_PATH_SEPARATOR_STR  "\\"
  
#elif defined (_UNIX)
  #define OS_PATH_SEPARATOR_CHAR '/'
  #define OS_PATH_SEPARATOR_STR  "/"
  
#elif defined (_MACINTOSH)
  #define OS_PATH_SEPARATOR_CHAR ':'
  #define OS_PATH_SEPARATOR_STR  ":"
#endif

//-----------------------------------------------------------------------------
// My defines
//-----------------------------------------------------------------------------
typedef PN_RESULT (*FPInit)(UINT32 access, IRMAFileResponse* pFileResp, IRMACommonClassFactory* pClassFactory);
typedef PN_RESULT (*FPGetFilename)(const char* pFileName);
typedef PN_RESULT (*FPRead)(UINT32 byteCount);
typedef PN_RESULT (*FPWrite)(IRMABuffer* pDataToWrite);
typedef PN_RESULT (*FPSeek)(UINT32 offset, BOOL bRelative);
typedef PN_RESULT (*FPAdvise)(UINT32 useage);
typedef PN_RESULT (*FPClose)();
typedef PN_RESULT (*FPSetRequest)(IRMARequest* pRequest);
typedef PN_RESULT (*FPGetRequest)(REF(IRMARequest*) pRequest);
typedef PN_RESULT (*FPStat)(IRMAFileStatResponse* pFileStatResponse);

typedef struct FPBindBlock
{
	FPBindBlock::FPBindBlock()
	{
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
	};

	FPInit			m_FPInit;
	FPGetFilename	m_FPGetFilename;
	FPRead			m_FPRead;
	FPWrite			m_FPWrite;
	FPSeek			m_FPSeek;
	FPAdvise		m_FPAdvise;
	FPClose			m_FPClose;
	FPSetRequest	m_FPSetRequest;
	FPGetRequest	m_FPGetRequest;
	FPStat			m_FPStat;
} FPBlock;
//-----------------------------------------------------------------------------


/****************************************************************************
 *
 *  LTRealFileObject Class
 *
 *  This class inherits the interfaces required to create a File Object,
 *  which is used by the File System plug-in to handle file I/O. This class
 *  implements the IRMAFileObject interface which handles the actual low
 *  level file access. The IRMARequestHandler interface is used to obtain
 *  the requested URL; while the IRMAFileExists interface determines if the
 *  requested file actually exists. Since we are using COM, this class also
 *  inherits COM's IUnknown interface to handle reference counting and
 *  interface query.
 */
class LTRealFileObject :  public IRMAFileObject,
                              public IRMADirHandler,
                              public IRMARequestHandler,
                              public IRMAFileExists,
			      public IRMAFileStat,
			      public IRMAGetFileFromSamePool
{
	public:

	LTRealFileObject
		(
		IRMACommonClassFactory* pClassFactory,
		char*                   pBasePath
		);
	~LTRealFileObject(void);


	/************************************************************************
	 *  IRMAFileObject Interface Methods                     ref:  rmafiles.h
	 */
	STDMETHOD(Init       ) (THIS_ UINT32 access,IRMAFileResponse* pFileResp);
	STDMETHOD(GetFilename) (THIS_ REF(const char*) pFileName);
	STDMETHOD(Read       ) (THIS_ UINT32 byteCount);
	STDMETHOD(Write      ) (THIS_ IRMABuffer* pDataToWrite);
	STDMETHOD(Seek       ) (THIS_ UINT32 offset, BOOL bRelative);
	STDMETHOD(Advise     ) (THIS_ UINT32 useage);
	STDMETHOD(Close      ) (THIS);


	/************************************************************************
	 *  IRMARequestHandler Interface Methods                 ref:  rmafiles.h
	 */
	STDMETHOD(SetRequest) (THIS_     IRMARequest*  pRequest);
	STDMETHOD(GetRequest) (THIS_ REF(IRMARequest*) pRequest);


	/************************************************************************
	 *  IRMAFileExists Interface Methods                     ref:  rmafiles.h
	 */
	STDMETHOD(DoesExist)
		(THIS_ 
		 const char*             pFilePathRMA,
		 IRMAFileExistsResponse* pFileResponse
		);


	/************************************************************************
	 *  IUnknown COM Interface Methods                          ref:  pncom.h
	 */
	STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
	STDMETHOD_(UINT32, AddRef ) (THIS);
	STDMETHOD_(UINT32, Release) (THIS);

	/************************************************************************
	 *  IRMAFileStat Interface Methods                       ref:  rmafiles.h
	 */
	STDMETHOD(Stat)
		(THIS_
		 IRMAFileStatResponse* pFileStatResponse
		);

	/************************************************************************
	 *  IRMADirHandler Interface Methods                       ref:  rmafiles.h
	 */
	STDMETHOD(InitDirHandler)
		(THIS_
		 IRMADirHandlerResponse* pDirResponse
		);

	/************************************************************************
	 *  IRMADirHandler Interface Methods                       ref:  rmafiles.h
	 */
	STDMETHOD(CloseDirHandler)	(THIS);

	/************************************************************************
	 *  IRMADirHandler Interface Methods                       ref:  rmafiles.h
	 */
	STDMETHOD(MakeDir)	(THIS);

	/************************************************************************
	 *  IRMADirHandler Interface Methods                       ref:  rmafiles.h
	 */
	STDMETHOD(ReadDir)	(THIS);

	/************************************************************************
	 *  IRMAGetFileFromSamePool Interface Methods            ref:  rmafiles.h
	 */
	STDMETHOD(GetFileObjectFromPool) 
				(THIS_
				IRMAGetFileFromSamePoolResponse* response
				);

	private:
	/****** Private Class Variables ****************************************/
	INT32                   m_RefCount;       // Object's reference count
	IRMACommonClassFactory* m_pClassFactory;  // Creates common RMA classes
	IRMAFileResponse*       m_pFileResponse;  // Provides completion notif.
	//FILE*                   m_pFile;          // Actual file pointer
	char*			m_pFilename;      // Object's copy of file name
	IRMARequest*            m_pRequest;       // Used to get requested URL
	char                    m_BasePath[1024]; // Platform's root path
	UINT32                  m_FileAccessMode; // Current file access mode
	IRMADirHandlerResponse* m_pDirResponse;   // Target for dir functions
#if defined (_WINDOWS ) && defined (_WIN32)
	LONG32			m_lFileHandle;    // Used for WIN32 ReadDir()
	struct _finddata_t	m_FileInfo;       // Used for WIN32 ReadDir()
#endif

public:
	FPInit			m_FPInit;
	FPGetFilename	m_FPGetFilename;
	FPRead			m_FPRead;
	FPWrite			m_FPWrite;
	FPSeek			m_FPSeek;
	FPAdvise		m_FPAdvise;
	FPClose			m_FPClose;
	FPSetRequest	m_FPSetRequest;
	FPGetRequest	m_FPGetRequest;
	FPStat			m_FPStat;
};

#endif // LTRealFileObject_H

