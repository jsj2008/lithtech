/****************************************************************************
 *
 *  $Id: filesys1.h,v 1.13 1998/06/13 03:08:23 glennh Exp $
 *
 *  Copyright (C) 1995,1996,1997 RealNetworks, Inc.
 *  All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary information of RealNetworks, Inc.,
 *  and is licensed subject to restrictions on use and distribution.
 * 
 *  filesys1.h
 *
 *  Example File System plug-in for the RealMedia Architecture (RMA).
 */
#ifndef LTRealFileSystem_H
#define LTRealFileSystem_H


/****************************************************************************
 * Includes
 */
#include <stdio.h>     /* FILE */
#if defined (_WINDOWS ) && defined (_WIN32)
#include <io.h>        /* struct _finddata_t, etc. */
#endif
#include "rmaplugn.h"  /* IRMAPlugin */
#include "rmafiles.h"  /* IRMAFileSystemObject */


/****************************************************************************
 * Constants
 */
#define LITH_PLUGIN_VERSION     0
#define LITH_DESCRIPTION       "Lithtech File System Plug-in"
#define LITH_COPYRIGHT         "(c) 2000,2001 Lithtech Inc., All rights reserved."
#define LITH_MORE_INFO_URL     "http://www.lithtech.com"
#define LITH_FILE_SYS_NAME     "lithtech_filesystem"
#define LITH_FILE_SYS_PROTOCOL "lithrez"
#define LITH_PROTOCOL_LENGTH    7


/****************************************************************************
 *
 *  LTRealFileSystem Class
 *
 *  This class inherits the interfaces required to create a File System
 *  plug-in. The IRMAFileSystemObject interface contains methods for
 *  initializing the File System object, and creating the File Object which
 *  handles the actual low level file access. All plug-ins also require the
 *  IRMAPlugin interface in order to handle fundamental plug-in operations.
 *  Since this is also a COM object, this class also inherits COM's IUnknown
 *  interface to handle reference counting and interface query.
 */
class LTRealFileSystem :  public IRMAFileSystemObject,
                              public IRMAPlugin
{
	public:
	 LTRealFileSystem(void);
	~LTRealFileSystem(void);


	/************************************************************************
	 *  IRMAFileSystemObject Interface Methods               ref:  rmafiles.h
	 */
	STDMETHOD(GetFileSystemInfo)
		(THIS_
		  REF(const char*) pShortName,
		  REF(const char*) pProtocol
		);
	STDMETHOD(InitFileSystem) (THIS_ IRMAValues*     pOptions);
	STDMETHOD(CreateFile    ) (THIS_ IUnknown**      ppFileObject);
	STDMETHOD(CreateDir     ) (THIS_ IUnknown**	 ppDirObject);


	/************************************************************************
	 *  IRMAPlugin Interface Methods                         ref:  rmaplugn.h
	 */
	STDMETHOD(GetPluginInfo)
		(THIS_ 
		  REF(BOOL)        bLoadMultiple,
		  REF(const char*) pDescription,
		  REF(const char*) pCopyright,
		  REF(const char*) pMoreInfoURL,
		  REF(UINT32)      versionNumber
		);
	STDMETHOD(InitPlugin) (THIS_ IUnknown* pRMACore);


	/************************************************************************
	 *  IUnknown COM Interface Methods                          ref:  pncom.h
	 */
	STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
	STDMETHOD_(UINT32, AddRef ) (THIS);
	STDMETHOD_(UINT32, Release) (THIS);


	private:
	/****** Private Class Variables ****************************************/
	INT32                    m_RefCount;       // Object's reference count
	IRMACommonClassFactory*  m_pClassFactory;  // Creates common RMA classes
	char                     m_BasePath[1024]; // Platform's root path

	/****** Private Static Class Variables *********************************/
	static const char*      zm_pDescription;
	static const char*      zm_pCopyright;
	static const char*      zm_pMoreInfoURL;
	static const char*	zm_pShortName;
	static const char*	zm_pProtocol;
};

#endif // LTRealFileSystem_H

