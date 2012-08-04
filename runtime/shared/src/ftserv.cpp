
#include "bdefs.h"
#include "ftserv.h"
#include "iltstream.h"
#include "de_world.h"
#include "packet.h"
#include "ftbase.h"
#include "netmgr.h"


// ----------------------------------------------------------------------- //
// Defines.
// ----------------------------------------------------------------------- //

// Transfer server states.
#define FTSTATE_NONE			0
#define FTSTATE_TRANSFERRING	1

// ----------------------------------------------------------------------- //
// Structures.
// ----------------------------------------------------------------------- //

// File info structure.
struct FTFile
{
	LTLink	m_Link;
	uint32	m_FileID;
	uint32	m_FileSize;
	char	*m_Filename;
	uint16	m_Flags;
};


// The file transfer server.  
struct FTServ
{
	CStringHolder		m_Strings;
	ObjectBank<FTFile>	m_FTFileBank;

	// All the files.  This list will be removed from as the 
	// files are successfully transfered.
	LTLink		m_Files;
	
	// How many files are there that the guy needs to have?
	int			m_nNeededFiles;

	// How many total files are there?
	int			m_nTotalFiles;

	// Current state (states defined in ftserv.cpp).
	int			m_State;

	// Flags for how we're operating.
	uint32		m_ServerFlags;

	// The maximum transfer rate.
	float		m_BytesPerSecond;

	// Time delta since the last packet was sent.
	float		m_TimeDelta;

	// The init structure is just copied over into here.
	FTSInitStruct	m_InitStruct;

	void		*m_UserData1;

	
	// Info about the current file transfer.
	ILTStream	*m_pCurFileStream;
	FTFile		*m_pCurFile;
	uint32		m_nUnverifiedBlocks;
	uint32		m_nBytesLeft;
};


// ----------------------------------------------------------------------- //
// Internal helpers.
// ----------------------------------------------------------------------- //

static FTFile* fts_FindFileToSend(FTServ *pServ)
{
	LTLink *pCur;
	FTFile *pFile;

	for(pCur=pServ->m_Files.m_pNext; pCur != &pServ->m_Files; pCur=pCur->m_pNext)
	{
		pFile = (FTFile*)pCur->m_pData;

		if( (pFile->m_Flags & FFLAG_CLIENTWANTS) )
		{
			return pFile;
		}
	}

	return LTNULL;
}


static FTFile* fts_FindFileByID(FTServ *pServ, uint16 fileID)
{
	LTLink *pCur;
	FTFile *pFile;

	for(pCur=pServ->m_Files.m_pNext; pCur != &pServ->m_Files; pCur=pCur->m_pNext)
	{
		pFile = (FTFile*)pCur->m_pData;
		if(pFile->m_FileID == fileID)
			return pFile;
	}

	return LTNULL;
}


// Remove the given file (and decrement the appropriate counts).
static void fts_RemoveFile(FTServ *pServ, FTFile *pFile)
{
	--pServ->m_nTotalFiles;
	if(pFile->m_Flags & FFLAG_NEEDED)
		--pServ->m_nNeededFiles;

	dl_Remove(&pFile->m_Link);
	pServ->m_FTFileBank.Free(pFile);
}


static void fts_MaybeSendDataBlock(FTServ *pServ)
{
	ASSERT(pServ->m_State == FTSTATE_TRANSFERRING);
	ASSERT(pServ->m_pCurFileStream);

	if(pServ->m_nUnverifiedBlocks == NUM_UNVERIFIED_BLOCKS)
	{
		// Ok, wait for an ack packet before sending more.
		return;
	}

	// Are we ready to send off another data block?
	uint32 sendSize = MAX_PACKET_LEN - 40;

	float bytesPerSecond = (float)sendSize / pServ->m_TimeDelta;
	if(bytesPerSecond > pServ->m_BytesPerSecond)
	{
		return;
	}

	uint8 tempData[MAX_PACKET_LEN];
	bool bDone = false;

	// Ok, send out a packet!
	CPacket_Write cDataPacket;
	cDataPacket.Writeuint8(STC_FILEBLOCK);

	if(sendSize >= pServ->m_nBytesLeft)
	{
		bDone = true;
		sendSize = pServ->m_nBytesLeft;
	}

	// Send the block.
	pServ->m_pCurFileStream->Read(tempData, sendSize);
	cDataPacket.WriteData(tempData, (uint16)sendSize);
	pServ->m_InitStruct.m_pNetMgr->SendPacket(CPacket_Read(cDataPacket), pServ->m_InitStruct.m_ConnID);

	// If this file transfer is done, then cleanup.
	if(bDone)
	{
		fts_RemoveFile(pServ, pServ->m_pCurFile);
		pServ->m_pCurFile = LTNULL;
		
		pServ->m_InitStruct.m_CloseFn(pServ, pServ->m_pCurFileStream);
		pServ->m_pCurFileStream = LTNULL;
	}

	++pServ->m_nUnverifiedBlocks;
	pServ->m_TimeDelta = 0;
}


inline int fts_FileDescLen(FTFile *pFile)
{
	return sizeof(uint16) + sizeof(uint32) + strlen(pFile->m_Filename);
}


inline void fts_WriteFileDesc(FTServ *pServ, FTFile *pFile, CPacket_Write &cPacket)
{
	if (cPacket.Empty())
		cPacket.Writeuint8(STC_FILEDESC);
	cPacket.Writeuint16((uint16)pFile->m_FileID);
	cPacket.Writeuint32(pFile->m_FileSize);
	cPacket.WriteString(pFile->m_Filename);
}


inline void fts_SendFileDesc(FTServ *pServ, FTFile *pFile)
{
	CPacket_Write cPacket;
	fts_WriteFileDesc(pServ, pFile, cPacket);
	pServ->m_InitStruct.m_pNetMgr->SendPacket(CPacket_Read(cPacket), pServ->m_InitStruct.m_ConnID);
}


// ----------------------------------------------------------------------- //
// Interface functions.
// ----------------------------------------------------------------------- //

FTServ *fts_Init(FTSInitStruct *pStruct, uint32 flags)
{
	FTServ *pRet;

	LT_MEM_TRACK_ALLOC(pRet = new FTServ,LT_MEM_TYPE_MISC);

	pRet->m_nNeededFiles = 0;
	pRet->m_nTotalFiles = 0;
	pRet->m_State = 0;
	pRet->m_ServerFlags = flags;
	pRet->m_TimeDelta = 0.0f;
	pRet->m_UserData1 = LTNULL;
	pRet->m_pCurFileStream = LTNULL;
	pRet->m_pCurFile = LTNULL;
	pRet->m_nUnverifiedBlocks = 0;
	pRet->m_nBytesLeft = 0;

	pRet->m_Strings.SetAllocSize(4096);
	LT_MEM_TRACK_ALLOC(pRet->m_FTFileBank.Init(128, 128), LT_MEM_TYPE_MISC);

	memcpy(&pRet->m_InitStruct, pStruct, sizeof(FTSInitStruct));
	dl_TieOff(&pRet->m_Files);
	pRet->m_State = FTSTATE_NONE;
	pRet->m_BytesPerSecond = 100000.0f; // Basically, send as fast as possible.

	// This was being reset causing models to be loaded more than once in single
	// player games... contact Peter Higley if this causes a problem
//	pRet->m_ServerFlags = 0;	(PLH 10/25/99)

	return (FTServ *)pRet;
}


void fts_Term(FTServ *pServ)
{
	fts_ClearFiles(pServ);
	fts_StopTransfer(pServ);
	delete pServ;
}


void* fts_GetUserData1(FTServ *pServ)
{
	if(pServ)
		return pServ->m_UserData1;
	else
		return LTNULL;
}


void fts_SetUserData1(FTServ *pServ, void *pUser)
{
	if(pServ)
		pServ->m_UserData1 = pUser;
}


void fts_SetServerFlags(FTServ *pServ, uint32 flags)
{
	if(pServ)
	{
		pServ->m_ServerFlags = flags;
	}
}


uint32 fts_GetServerFlags(FTServ *pServ)
{
	if(pServ)
		return pServ->m_ServerFlags;
	else
		return 0;
}


uint32 fts_GetNumNeededFiles(FTServ *pServ)
{
	if(pServ)
		return pServ->m_nNeededFiles;
	else
		return 0;
}


uint32 fts_GetNumTotalFiles(FTServ *pServ)
{
	if(pServ)
		return pServ->m_nTotalFiles;
	else
		return 0;
}


int fts_AddFile(FTServ *pServ, char *pFilename, uint32 fileSize, uint32 fileID, uint16 flags)
{
	FTFile *pFile;

	if(!pServ)
		return 0;
	
	LT_MEM_TRACK_ALLOC(pFile = pServ->m_FTFileBank.Allocate(), LT_MEM_TYPE_MISC);
	pFile->m_Link.m_pData = pFile;
	pFile->m_Flags = flags;
	pFile->m_FileID = fileID;
	pFile->m_FileSize = fileSize;
	pFile->m_Filename = pFilename;

	if(pFile->m_Flags & FFLAG_NEEDED)
	{
		// Needed files go first in the list.
		dl_Insert(&pServ->m_Files, &pFile->m_Link);
		++pServ->m_nNeededFiles;
	}
	else
	{
		dl_Insert(pServ->m_Files.m_pPrev, &pFile->m_Link);
	}

	// Tell the client about this file.
	if(!(flags & FFLAG_SENDWAIT))
	{
		fts_SendFileDesc(pServ, pFile);
	}

	++pServ->m_nTotalFiles;

	return 1;
}


void fts_FlushAddedFiles(FTServ *pServ)
{
	if(!pServ)
		return;

	CPacket_Write cPacket;

	int nSentFiles = 0, nNotSentFiles = 0;

	for(LTLink *pCur = pServ->m_Files.m_pNext; pCur != &pServ->m_Files; pCur = pCur->m_pNext)
	{
		FTFile *pFile = (FTFile*)pCur->m_pData;
		if(pFile->m_Flags & FFLAG_SENDWAIT)
		{
			fts_WriteFileDesc(pServ, pFile, cPacket);
			pFile->m_Flags &= ~FFLAG_SENDWAIT;
			++nSentFiles;
		}
		else
			++nNotSentFiles;
	}

	// Flush it out..
	if (!cPacket.Empty())
	{
		pServ->m_InitStruct.m_pNetMgr->SendPacket(CPacket_Read(cPacket), pServ->m_InitStruct.m_ConnID);
	}

	ASSERT((nSentFiles + nNotSentFiles) == pServ->m_nTotalFiles);
}


void fts_ClearFiles(FTServ *pServ)
{
	LTLink *pCur, *pNext;
	FTFile *pFile;

	pCur = pServ->m_Files.m_pNext;
	while(pCur != &pServ->m_Files)
	{
		pNext = pCur->m_pNext;
		
		// Free it if we're not transferring it.
		pFile = (FTFile*)pCur->m_pData;
		if(!(pFile->m_Flags & FFLAG_TRANSFERRING))
		{		
			fts_RemoveFile(pServ, pFile);
		}

		pCur = pNext;
	}
}


void fts_StopTransfer(FTServ *pServ)
{
	if (!pServ || (pServ->m_State != FTSTATE_TRANSFERRING))
		return;

	CPacket_Write cPacket;
	cPacket.Writeuint8(STC_CANCELFILETRANSFER);
	pServ->m_InitStruct.m_pNetMgr->SendPacket(CPacket_Read(cPacket), pServ->m_InitStruct.m_ConnID);
	pServ->m_InitStruct.m_CloseFn(pServ, pServ->m_pCurFileStream);
	pServ->m_pCurFile->m_Flags &= ~FFLAG_TRANSFERRING;
	pServ->m_pCurFile = LTNULL;
	pServ->m_pCurFileStream = LTNULL;
	pServ->m_State = FTSTATE_NONE;
}


bool fts_ProcessPacket(FTServ *pServ, const CPacket_Read &cPacket)
{
	CPacket_Read cPacket_Input(cPacket);

	cPacket_Input.SeekTo(0);
	uint8 nPacketID = cPacket_Input.Readuint8();

	switch (nPacketID)
	{
		case CTS_FILESTATUS :
		{
			while (!cPacket_Input.EOP())
			{
				uint16 nFileID = cPacket_Input.Readuint16();
				bool bClientHasFile = ((nFileID & 0x8000) != 0);
				nFileID &= ~0x8000;

				FTFile *pFile = fts_FindFileByID(pServ, nFileID);
				if(pFile)
				{
					if(bClientHasFile)
						fts_RemoveFile(pServ, pFile);
					else		
						pFile->m_Flags |= FFLAG_CLIENTWANTS;
				}
			}
			break;
		}
		case CTS_DATARECEIVED :
		{
			if(pServ->m_State == FTSTATE_TRANSFERRING)
				pServ->m_nUnverifiedBlocks = 0;

			break;
		}
		default : 
			return false;
	}

	return true;
}


void fts_Update(FTServ *pServ, float timeDelta)
{
	FTFile *pFile;
	ILTStream *pStream;
	int todo;
	

	if(!pServ)
		return;
	
	// What's our state?
	if(pServ->m_State == FTSTATE_NONE)
	{
		// Do we have any files that need to be sent?
		pFile = fts_FindFileToSend(pServ);
		if(pFile)
		{
			pServ->m_TimeDelta = 0.0f;

			// If they don't want us to send files at all right now, don't,
			if(pServ->m_ServerFlags & FTSFLAG_DONTSENDANYTHING)
				return;
			
			// If it only wants us to send needed files, then don't do anything if the
			// file isn't needed.
			if(pServ->m_ServerFlags & FTSFLAG_ONLYSENDNEEDED)
			{
				if(!(pFile->m_Flags & FFLAG_NEEDED))
				{
					return;
				}
			}			   

			pStream = pServ->m_InitStruct.m_OpenFn(pServ, pFile->m_Filename);
			if(pStream)
			{
				// Cooooool, start the transfer.
				pFile->m_Flags |= FFLAG_TRANSFERRING;
				pServ->m_pCurFile = pFile;
				pServ->m_pCurFileStream = pStream;
				pServ->m_State = FTSTATE_TRANSFERRING;
				pServ->m_nUnverifiedBlocks = 0;
				pServ->m_nBytesLeft = pFile->m_FileSize;
			}
			else
			{
				todo = pServ->m_InitStruct.m_CantOpenFileFn(pServ, pFile->m_Filename);
				if(todo == TODO_REMOVEFILE)
				{
					dl_Remove(&pFile->m_Link);
					pServ->m_FTFileBank.Free(pFile);
				}
				else if(todo == TODO_RETURN)
				{
					return;
				}
			}
		}
	}
	else if(pServ->m_State == FTSTATE_TRANSFERRING)
	{
		pServ->m_TimeDelta += timeDelta;

		fts_MaybeSendDataBlock(pServ);
	}
}




