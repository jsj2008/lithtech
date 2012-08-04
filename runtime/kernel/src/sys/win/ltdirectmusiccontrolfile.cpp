/****************************************************************************
;
;	MODULE:		LTDirectMusicControlFile (.CPP)
;
;	PURPOSE:	Control file reader which uses DStreams to do the file reading.
;
;	HISTORY:	1999 [blb]
;
;	NOTICE:		Copyright (c) 1999,2000 
;				Monolith Procutions, Inc.
;
***************************************************************************/

#include "bdefs.h"

#include "ltdirectmusiccontrolfile.h"

#ifdef NOLITHTECH
#include "lith.h"
#include "lithtmpl.h"
#include "ltbasedefs.h"
#endif

#ifndef NOLITHTECH


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);


///////////////////////////////////////////////////////////////////////////////////////////
// read in data from a file
///////////////////////////////////////////////////////////////////////////////////////////
BOOL CControlFileMgrDStream::FileOpen(const char* sName)
{
	ILTStream *pFileStream;
	FileRef ref;
	ref.m_FileType = FILE_ANYFILE;
	ref.m_pFilename = sName;

	// open the file
	pFileStream = client_file_mgr->OpenFile(&ref);
	if (pFileStream == LTNULL) 
	{
		return false;
	}

	// make some data to hold the file data (these files are small so just load it all in)
	LT_MEM_TRACK_ALLOC(m_pData = new char[pFileStream->GetLen()],LT_MEM_TYPE_MUSIC);
	if (m_pData == LTNULL) 
	{
		pFileStream->Release();
		return false;
	}

	// read in the data
	if (pFileStream->Read( m_pData, pFileStream->GetLen()) != LT_OK)
	{
		pFileStream->Release();
		delete [] m_pData;
		return false;
	}

	// set current position to start of data
	m_pPos = m_pData;

	// figure out last position in data
	m_pEnd = m_pData + pFileStream->GetLen() - 1;

	// close the file
	pFileStream->Release();

	// we succeeded
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// delete file data
///////////////////////////////////////////////////////////////////////////////////////////
void CControlFileMgrDStream::FileClose()
{
	// free the file data
	if (m_pData != LTNULL) delete [] m_pData;
}

	
///////////////////////////////////////////////////////////////////////////////////////////
// get next character
///////////////////////////////////////////////////////////////////////////////////////////
int CControlFileMgrDStream::FileGetChar()
{
	// make sure file data is not null
	if (m_pData == LTNULL) return 0;

	// make sure we are not past the end of the data
	if (FileEOF()) return 0;

	// get the value to return
	int nRetVal = *m_pPos;

	// increment the position
	m_pPos++;

	// return the value 
	return nRetVal;
}

	
///////////////////////////////////////////////////////////////////////////////////////////
// return true if we are at the end of the data
///////////////////////////////////////////////////////////////////////////////////////////
BOOL CControlFileMgrDStream::FileEOF()
{
	// make sure file data is not LTNULL
	if (m_pData == LTNULL) return false;

	// return true if we are past the end of the file
	if (m_pPos > m_pEnd) return true;

	// return false if not past end of file
	else return false;
}


// BOGUS MUSIC FILE LOAD TEST THROUGH DSTREAMS
// This is a test remove this function. TO DO!!!
void FileReadTest()
{
	char* pFileName = "\\directmusic\\sanity6\\sanity6script.txt";

	FileRef ref;
	ref.m_FileType = FILE_ANYFILE;
	ref.m_pFilename = pFileName;

	ILTStream *pFileStream = client_file_mgr->OpenFile(&ref);

	if (pFileStream != LTNULL)
	{
		// seek to start of file
		pFileStream->SeekTo( 0 );

		if (pFileStream->GetLen() > 0)
		{
			char* sData;
			LT_MEM_TRACK_ALLOC(sData = new char[pFileStream->GetLen()],LT_MEM_TYPE_MUSIC);

			if (sData != LTNULL)
			{
				// read in the data
				pFileStream->Read( sData, pFileStream->GetLen());

				delete [] sData;
			}
		}

		// Close the file
		pFileStream->Release();
	}
}

#endif

#ifdef NOLITHTECH
///////////////////////////////////////////////////////////////////////////////////////////
// read in data from a file
///////////////////////////////////////////////////////////////////////////////////////////
BOOL CControlFileMgrRezFile::FileOpen(const char* sName)
{
	// make sure rez file is open
	if (!m_RezMgr.IsOpen()) return false;

	// get the rez item
	m_pItem = m_RezMgr.GetRezFromDosPath(sName);
	if (m_pItem == LTNULL) return false;

	// load the data
	m_pData = (char*)m_pItem->Load();
	if (m_pData == LTNULL) return false;

	// set current position to start of data
	m_pPos = m_pData;

	// figure out last position in data
	m_pEnd = m_pData + m_pItem->GetSize() - 1;

	// we succeeded
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// delete file data
///////////////////////////////////////////////////////////////////////////////////////////
void CControlFileMgrRezFile::FileClose()
{
	// free the file data
	if (m_pItem != LTNULL) m_pItem->UnLoad();
	m_pItem = LTNULL;
}

	
///////////////////////////////////////////////////////////////////////////////////////////
// get next character
///////////////////////////////////////////////////////////////////////////////////////////
int CControlFileMgrRezFile::FileGetChar()
{
	// make sure file data is not null
	if (m_pData == LTNULL) return 0;

	// make sure we are not past the end of the data
	if (FileEOF()) return 0;

	// get the value to return
	int nRetVal = *m_pPos;

	// increment the position
	m_pPos++;

	// return the value 
	return nRetVal;
}

	
///////////////////////////////////////////////////////////////////////////////////////////
// return true if we are at the end of the data
///////////////////////////////////////////////////////////////////////////////////////////
BOOL CControlFileMgrRezFile::FileEOF()
{
	// make sure file data is not LTNULL
	if (m_pData == LTNULL) return false;

	// return true if we are past the end of the file
	if (m_pPos > m_pEnd) return true;

	// return false if not past end of file
	else return false;
}
#endif


