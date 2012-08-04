/****************************************************************************
;
;	MODULE:		LTRealFileObject (.H)
;
;	PURPOSE:	Implement Real support in LithTech engine
;
;	HISTORY:	1-20-2001 [mds] File created.
;
;	NOTICE:		Copyright (c) 2001 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRealFileObject_H
#define LTRealFileObject_H

#define LITHTECH_ESD_INC 1
#include "lith.h"
#include "bdefs.h"
#undef LITHTECH_ESD_INC
#include "iltesd.h"
#include "pnwintyp.h"
#include "pncom.h"
#include "rmapckts.h"
#include "rmacomm.h"
#include "rmamon.h"
#include "rmafiles.h"
#include "rmaengin.h"
#include "rmacore.h"
#include "rmaclsnk.h"
#include "rmaerror.h"
#include "rmaauth.h"
#include "rmaausvc.h"
#include "rmawin.h"

#include "LTRClientContext.h"
#include "LTRAudioDevice.h"
#include "LTRClientEngineSetup.h"

//-----------------------------------------------------------------------------
#define LITH_PLUGIN_VERSION     0
#define LITH_DESCRIPTION       "Lithtech File System Plug-in"
#define LITH_COPYRIGHT         "(c) 2000,2001 Lithtech Inc., All rights reserved."
#define LITH_MORE_INFO_URL     "http://www.lithtech.com"
#define LITH_FILE_SYS_NAME     "lithtech_filesystem"
#define LITH_FILE_SYS_PROTOCOL "lithrez"
#define LITH_PROTOCOL_LENGTH    7
//-----------------------------------------------------------------------------

// File system hooks
//-----------------------------------------------------------------------------
typedef PN_RESULT (*FPInit)(UINT32 access, IRMAFileResponse* pFileResponse, IRMACommonClassFactory* pClassFactory);
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
} Block;

typedef void (*FPBindFunctions)(FPBindBlock* pBlock);
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Main class for CLTRealFileObject
//-----------------------------------------------------------------------------
class CLTRealFileObject
{
public:
	// Default constructor
	CLTRealFileObject();

	// Default destructor (calls Term if it has not been called)
	~CLTRealFileObject();

	// File system primatives
	static PN_RESULT Init(UINT32 access, IRMAFileResponse* pFileResponse, IRMACommonClassFactory* pClassFactory);
	static PN_RESULT GetFilename(const char* pFileName);
	static PN_RESULT Read(UINT32 byteCount);
	static PN_RESULT Write(IRMABuffer* pDataToWrite);
	static PN_RESULT Seek(UINT32 offset, BOOL bIsRelative);
	static PN_RESULT Advise(UINT32 useage);
	static PN_RESULT Close();
	static PN_RESULT SetRequest(IRMARequest* pRequest);
	static PN_RESULT GetRequest(REF(IRMARequest*) pRequest);
	static PN_RESULT Stat(IRMAFileStatResponse* pFileStatResponse);

	// Use this class as a singleton
	static CLTRealFileObject& Instance()
	{
		static CLTRealFileObject Instance;
		return Instance;
	}

protected:
	static ILTStream*				m_pStream;
	static char*					m_pFilename;		// Object's copy of file name
	static IRMARequest*				m_pRequest;			// Used to get requested URL
	static IRMAFileResponse*		m_pFileResponse;	// Provides completion notification
	static IRMACommonClassFactory*	m_pClassFactory;	// Creates common RMA classes
};

inline static CLTRealFileObject& CLTRealFileObject()
{
	return CLTRealFileObject::Instance();
}

#endif // LTRealFileObject_H
#endif // LITHTECH_ESD