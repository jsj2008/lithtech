/****************************************************************************
;
;	MODULE:		LTRealFileObject (.CPP)
;
;	PURPOSE:	Implement Real support in LithTech engine
;
;	HISTORY:	1-20-2001 [mds] File created.
;
;	NOTICE:		Copyright (c) 2001 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrealfileobject.h"
#include "ltrconout.h"

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr* client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

ILTStream*				CLTRealFileObject::m_pStream;
char*					CLTRealFileObject::m_pFilename;
IRMARequest*			CLTRealFileObject::m_pRequest;
IRMAFileResponse*		CLTRealFileObject::m_pFileResponse;
IRMACommonClassFactory*	CLTRealFileObject::m_pClassFactory;

//-----------------------------------------------------------------------------
// CLTRealFileObject member functions
//-----------------------------------------------------------------------------
CLTRealFileObject::CLTRealFileObject()
{
	m_pStream = NULL;
	m_pFilename = NULL;
	m_pRequest = NULL;
	m_pFileResponse = NULL;
	m_pClassFactory = NULL;
}

//-----------------------------------------------------------------------------
CLTRealFileObject::~CLTRealFileObject()
{
	if (m_pStream)
	{
		m_pStream->Release();
		m_pStream = NULL;
	}

	if (m_pFilename)
	{
		delete[] m_pFilename;
		m_pFilename = NULL;
	}
}

//-----------------------------------------------------------------------------
PN_RESULT CLTRealFileObject::Init(UINT32 access, IRMAFileResponse* pFileResponse, IRMACommonClassFactory* pClassFactory)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealFileObject::Init");

	// Check response
	if (!pFileResponse || !pClassFactory)
		return PNR_INVALID_PARAMETER;

	// Signify that we need to keep a reference to this object
	if (!m_pClassFactory)
	{
		m_pClassFactory = pClassFactory;
		m_pClassFactory->AddRef();
	}

	//
	// Associate this File Object with a File Response object for completion
	// notification.
	//

	// Release any previous File Response objects
	if (m_pFileResponse != NULL)
		m_pFileResponse->Release();

	m_pFileResponse = pFileResponse;
	m_pFileResponse->AddRef();

	//
	// Open the file and notify File Response when complete
	//

	PN_RESULT result = PNR_FAILED;
	if (client_file_mgr && m_pFilename)
	{
		// If we have an open stream, close it...
		if (m_pStream)
		{
			m_pStream->Release();
			m_pStream = NULL;
		}

		// Try and get the resource out of the rez file
		FileRef fileRef;
		fileRef.m_pFilename = m_pFilename;
		m_pStream = client_file_mgr->OpenFile(&fileRef);
		if (m_pStream)
			result = PNR_OK;
	}

	m_pFileResponse->InitDone(result);

	return result;
}

//-----------------------------------------------------------------------------
PN_RESULT CLTRealFileObject::GetFilename(const char* pFileName)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealFileObject::GetFilename");

	if (m_pFilename)
	{
		pFileName = m_pFilename;
		return PNR_OK;
	}

	return PNR_FAIL;
}

//-----------------------------------------------------------------------------
PN_RESULT CLTRealFileObject::Read(UINT32 byteCount)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealFileObject::Read");

	if (!m_pClassFactory || !m_pStream)
		return PNR_UNEXPECTED;

	// Create a buffer object to store the data which is to be read
	IRMABuffer* pBuffer = NULL;
	m_pClassFactory->CreateInstance(CLSID_IRMABuffer, (void**)&pBuffer);
	if (pBuffer)
	{
		// Calculate how much data we have left
		unsigned int ulPos = 0;
		unsigned int ulLen = 0;
		m_pStream->GetPos(&ulPos);
		m_pStream->GetLen(&ulLen);

		uint32 bytesLeft = ulLen - ulPos;
		uint32 actualByteCount = MIN(byteCount, bytesLeft);

		// Set the size of the buffer
		PN_RESULT sizeResult = pBuffer->SetSize(actualByteCount);
		if (PNR_OK != sizeResult)
			return sizeResult;

		// Read from the file directly into the buffer object
		LTRESULT result = m_pStream->Read(pBuffer->GetBuffer(), actualByteCount);
		if (LT_OK != result)
		{
			m_pStream->Release();
			m_pStream = NULL;
		}

		// Notify the caller that the read is done
		PN_RESULT readResult = actualByteCount > 0 ? PNR_OK : PNR_FAILED;
		m_pFileResponse->ReadDone(readResult, pBuffer);

		// Release the buffer object since we are done with it
		pBuffer->Release();
	}
	else
		return PNR_OUTOFMEMORY;

	return PNR_OK;
}

//-----------------------------------------------------------------------------
PN_RESULT CLTRealFileObject::Write(IRMABuffer* pDataToWrite)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealFileObject::Write");

	PN_RESULT pResult = PNR_FAIL;

	return pResult;
}

//-----------------------------------------------------------------------------
PN_RESULT CLTRealFileObject::Seek(UINT32 offset, BOOL bIsRelative)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealFileObject::Seek");

	PN_RESULT pResult = PNR_OK;

	if (m_pStream)
	{
		unsigned int ulPos = 0;
		unsigned int ulLen = 0;
		m_pStream->GetPos(&ulPos);
		m_pStream->GetLen(&ulLen);

		offset = MIN(ulLen, offset);

		if (bIsRelative)
			m_pStream->SeekTo(ulPos + offset);
		else
			m_pStream->SeekTo(offset);
	}

	m_pFileResponse->SeekDone(pResult);

	return pResult;
}

//-----------------------------------------------------------------------------
PN_RESULT CLTRealFileObject::Advise(UINT32 useage)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealFileObject::Advise");

	PN_RESULT pResult = PNR_OK;

	return pResult;
}

//-----------------------------------------------------------------------------
PN_RESULT CLTRealFileObject::Close()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealFileObject::Close");

	if (m_pRequest != NULL)
	{
		m_pRequest->Release();
		m_pRequest = NULL;
	}

	if (m_pFilename)
	{
		delete[] m_pFilename;
		m_pFilename = NULL;
	}

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

//-----------------------------------------------------------------------------
PN_RESULT CLTRealFileObject::SetRequest(IRMARequest* pRequest)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealFileObject::SetRequest");

	// Release any previous request objects
	if (m_pRequest != NULL)
		m_pRequest->Release();

	// Store a reference to the object
	m_pRequest = pRequest;
	m_pRequest->AddRef();

	// Get the file name
	char* pFilename = NULL;
	if (PNR_OK != m_pRequest->GetURL(pFilename))
		return PNR_FAIL;

	// Strip protocol
	uint32 protocolLength = LITH_PROTOCOL_LENGTH + 1; // Add 1 for the colon
	char* pProtocolString;
	LT_MEM_TRACK_ALLOC(pProtocolString = new char[protocolLength + 1],LT_MEM_TYPE_MISC);
	strcpy(pProtocolString, LITH_FILE_SYS_PROTOCOL);
	strcat(pProtocolString, ":");
	if (0 == _strnicmp(pFilename, pProtocolString, protocolLength))
	{
		strcpy(pFilename, &pFilename[protocolLength]);

		// Strip leading URL slashes
		uint32 index = 0;
		while ('/' == pFilename[index])
			index++;

		strcpy(pFilename, &pFilename[index]);
	}

	delete[] pProtocolString;

	// Replace path slashes with platform specific path separators
	for (uint32 index = 0; index < strlen(pFilename); index++)
	{
		if ('/' == pFilename[index])
			pFilename[index] = '\\';
	}

	// Do we have a filename allocated?
	if (m_pFilename)
	{
		delete[] m_pFilename;
		m_pFilename = NULL;
	}

	// Make a copy
	LT_MEM_TRACK_ALLOC(m_pFilename = new char[strlen(pFilename) + 1],LT_MEM_TYPE_MISC);
	strcpy(m_pFilename, pFilename);

	return PNR_OK;
}

//-----------------------------------------------------------------------------
PN_RESULT CLTRealFileObject::GetRequest(REF(IRMARequest*) pRequest)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealFileObject::GetRequest");

	// Return a reference to the object
	pRequest = m_pRequest;
	if (pRequest != NULL)
		pRequest->AddRef();

	return PNR_OK;
}

//-----------------------------------------------------------------------------
PN_RESULT CLTRealFileObject::Stat(IRMAFileStatResponse* pFileStatResponse)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealFileObject::Stat");

	unsigned int ulLen = 0;
	m_pStream->GetLen(&ulLen);

    struct stat StatBuffer;
	memset(&StatBuffer, 0, sizeof(struct stat));

	// The size of the file is what we need to report
	StatBuffer.st_size = ulLen;
	
	pFileStatResponse->StatDone(PNR_OK, 
				StatBuffer.st_size, 
				StatBuffer.st_ctime, 
				StatBuffer.st_atime,
				StatBuffer.st_mtime,
				StatBuffer.st_mode);

	return PNR_OK;
}

#endif // LITHTECH_ESD
