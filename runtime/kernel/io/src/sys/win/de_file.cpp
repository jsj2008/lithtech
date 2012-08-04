// ------------------------------------------------------------------ //
// Includes..
// ------------------------------------------------------------------ //

#include "bdefs.h"

#ifndef __STRING_H__
#include <string.h>
#define __STRING_H__
#endif

#ifndef __IO_H__
#include <io.h>
#define __IO_H__
#endif

#ifndef __STDIO_H__
#include <stdio.h>
#define __STDIO_H__
#endif

#ifndef __DE_FILE_H__
#include "de_file.h"
#endif

#ifndef __DE_MEMORY_H__
#include "de_memory.h"
#endif

#ifndef __GENLTSTREAM_H__
#include "genltstream.h"
#endif

#ifndef __DUTIL_H__
#include "dutil.h"
#endif

#ifndef __DSYS_INTERFACE_H__
#include "dsys_interface.h"
#endif

#ifndef __SYSLTHREAD_H__
#include "syslthread.h"
#endif

#include "syscounter.h"


// console output of file access
// 0 - no output (default)
// 1 - display file open calls
// 2 - display file open and close calls
// 3 - display file open and close calls
// 4 - display file open and close and read calls
extern int32 g_CV_ShowFileAccess;

#ifndef __REZMGR_H__
#include "rezmgr.h"
#endif

// define this if you want all RezMgr reads to be made thread safe!
// (This currently doesn't make anythnig else thread safe including using a Stream in two 
//  threads at once!)
#define LT_FILE_THREADSAFE


// PlayDemo profile info.
uint32 g_PD_FOpen=0;


// ------------------------------------------------------------------ //
// Structures and defines.
// ------------------------------------------------------------------ //

CRezItm* g_pDeFileLastRezItm;
uint32 g_nDeFileLastRezPos;


struct FileTree
{
	TreeType	m_TreeType;
	CRezMgr*	m_pRezMgr;
	
	#ifdef LT_FILE_THREADSAFE
	CRITICAL_SECTION m_CriticalSection;
	#endif
	
	// Directory name if this is a DosTree (like e:/dedit/riot)
	// Rezfile name if this is a RezFileTree (like e:/dedit/riot/riot.rez)
	char		m_BaseName[1];
};


class BaseFileStream : public CGenLTStream
{
public:

			BaseFileStream()
			{
				m_FileLen = 0;
				m_pFile = LTNULL;
				m_pTree = LTNULL;
				m_ErrorStatus = 0;
				m_pRezItm = LTNULL;
			}

	virtual	~BaseFileStream()
	{
		if (g_CV_ShowFileAccess >= 2)
		{
			dsi_ConsolePrint("close stream %p", this);
		}

		if(m_pTree->m_TreeType == DosTree && m_pFile)
		{
			fclose(m_pFile);
		}
	}

	LTRESULT ErrorStatus()
	{
		return m_ErrorStatus ? LT_ERROR : LT_OK;
	}

	LTRESULT GetLen(uint32 *len)
	{
		*len = m_FileLen;
		return LT_OK;
	}

	LTRESULT Write(const void *pData, uint32 dataLen)
	{
		// Can't write to these streams.
		return LT_ERROR;
	}

	uint32		m_FileLen;		// Stored when the file is opened.
	FILE		*m_pFile;
	FileTree	*m_pTree;
	int			m_ErrorStatus;
	CRezItm*	m_pRezItm;
};


class DosFileStream : public BaseFileStream
{
public:

	void	Release();

	LTRESULT GetPos(uint32 *pos)
	{
		*pos = (uint32)(ftell(m_pFile));
		return LT_OK;
	}
	
	LTRESULT	SeekTo(uint32 offset)
	{
		if(fseek(m_pFile, offset, SEEK_SET) == 0)
		{
			return LT_OK;
		}
		else
		{
			m_ErrorStatus = 1;
			return LT_ERROR;
		}
	}

	LTRESULT Read(void *pData, uint32 size)
	{
		if(!m_ErrorStatus && (fread(pData, 1, size, m_pFile) == size))
		{
			return LT_OK;
		}

		//encountered an error while reading
		memset(pData, 0, size);
		m_ErrorStatus = 1;
		return LT_ERROR;
	}
};

class RezFileStream : public BaseFileStream
{
public:

	RezFileStream() :
	  m_SeekOffset(0)
	{
	}

	void		Release();

	LTRESULT	GetPos(uint32 *pos)
	{
		*pos = m_SeekOffset;
		return LT_OK;
	}

	LTRESULT	SeekTo(uint32 offset)
	{
		if(m_pRezItm->Seek(offset))
		{
			m_SeekOffset = offset;
			return LT_OK;
		}
		else
		{
			m_ErrorStatus = 1;
			return LT_ERROR;
		}
	}

	LTRESULT Read(void *pData, uint32 size)
	{
		size_t sizeRead;

		if(size != 0)
		{
			#ifdef LT_FILE_THREADSAFE
			EnterCriticalSection(&m_pTree->m_CriticalSection);
			#endif
			
			if ((g_pDeFileLastRezItm == m_pRezItm) && (g_nDeFileLastRezPos == m_SeekOffset))
			{
				sizeRead = m_pRezItm->Read(pData, size);
			}
			else
			{
				sizeRead = m_pRezItm->Read(pData, size, m_SeekOffset);
			}
			
			#ifdef LT_FILE_THREADSAFE
			LeaveCriticalSection(&m_pTree->m_CriticalSection);
			#endif
			
			m_SeekOffset += sizeRead;
			g_pDeFileLastRezItm = m_pRezItm;
			g_nDeFileLastRezPos = m_SeekOffset;
			if(sizeRead != size)
			{
				memset(pData, 0, size);
				m_ErrorStatus = 1;
				return LT_ERROR;
			}
		}
		return LT_OK;
	}

	bool IsRawFileInfoAvailable()
	{
	}

	LTRESULT GetRawFileInfo(char* sFileName, uint32* nPos)
	{
	}

	uint32		m_SeekOffset;	// Seek offset (used in rezfiles) (NOTE: in a rezmgr rez file this is the current position inside the resource).

};



static ObjectBank<DosFileStream, LCriticalSection> g_DosFileStreamBank(8, 8);
static ObjectBank<RezFileStream, LCriticalSection> g_RezFileStreamBank(8, 8);


void DosFileStream::Release()
{
	g_DosFileStreamBank.Free(this);
}

void RezFileStream::Release()
{
	g_RezFileStreamBank.Free(this);
}



// LTFindInfo::m_pInternal..
struct LTFindData
{
	FileTree		*m_pTree;
	_finddata_t		m_Data;
	long			m_Handle;
	CRezDir*		m_pCurDir;
	CRezTyp*		m_pCurTyp;
	CRezItm*		m_pCurItm;
	CRezDir*		m_pCurSubDir;
};



// ------------------------------------------------------------------ //
// Interface functions.
// ------------------------------------------------------------------ //

void df_Init()
{
	g_pDeFileLastRezItm = LTNULL;
	g_nDeFileLastRezPos = 0;
}

void df_Term()
{
}

int df_OpenTree(const char *pName, HLTFileTree *&pTreePointer)
{
	_finddata_t data;
	long handle, allocSize;
	FileTree *pTree;

	
	pTreePointer = NULL;

	// See if it exists..
	handle = _findfirst(pName, &data);
	if(handle == -1)
		return -1;

	_findclose(handle);

	
	allocSize = sizeof(FileTree) + strlen(pName);
	if(data.attrib & _A_SUBDIR)
	{
		// Ok, setup a dos tree.
		LT_MEM_TRACK_ALLOC(pTree = (FileTree*)dalloc_z(allocSize),LT_MEM_TYPE_FILE);
		pTree->m_TreeType = DosTree;
		pTree->m_pRezMgr = LTNULL;
	}
	else
	{
		LT_MEM_TRACK_ALLOC(pTree = (FileTree*)dalloc_z(allocSize),LT_MEM_TYPE_FILE);

		pTree->m_TreeType = RezFileTree;

		LT_MEM_TRACK_ALLOC(pTree->m_pRezMgr = new CRezMgr,LT_MEM_TYPE_FILE);
		if (pTree->m_pRezMgr == LTNULL)
			return -2;
		
		if(!pTree->m_pRezMgr->Open(pName))
		{
			delete pTree->m_pRezMgr;
			dfree(pTree);
			return -2;
		}

		pTree->m_pRezMgr->SetDirSeparators("\\/");
		g_pDeFileLastRezItm = LTNULL;
		g_nDeFileLastRezPos = 0;

#ifdef LT_FILE_THREADSAFE
		InitializeCriticalSection(&pTree->m_CriticalSection);
#endif
	}

	strcpy(pTree->m_BaseName, pName);
	pTreePointer = (HLTFileTree *)pTree;
	
	return 0;
}


void df_CloseTree(HLTFileTree *hTree)
{
	FileTree *pTree;

	pTree = (FileTree*)hTree;
	if(!pTree)
		return;

	if (pTree->m_pRezMgr != LTNULL)
	{
		delete pTree->m_pRezMgr;
		pTree->m_pRezMgr = LTNULL;
#ifdef LT_FILE_THREADSAFE
		DeleteCriticalSection(&pTree->m_CriticalSection);
#endif
	}
	g_pDeFileLastRezItm = LTNULL;
	g_nDeFileLastRezPos = 0;

	dfree(pTree);
}


TreeType df_GetTreeType(HLTFileTree *hTree)
{
	FileTree *pTree;

	pTree = (FileTree*)hTree;
	if(!pTree)
		return DosTree;

	return pTree->m_TreeType;
}


bool df_GetFileInfo(HLTFileTree *hTree, const char *pName, LTFindInfo *pInfo) {
	FileTree *pTree;
	_finddata_t data;
	long handle, curRet;
	char fullName[500];
	CRezItm* pRezItm;

	pTree = (FileTree*)hTree;
	if(!pTree)
		return false;

	if(pTree->m_TreeType == DosTree)
	{
		LTSNPrintF(fullName, sizeof(fullName), "%s\\%s", pTree->m_BaseName, pName);

		handle = _findfirst(fullName, &data);
		curRet = handle;
		while(curRet != -1)
		{
			if(!(data.attrib & _A_SUBDIR))
			{
				LTStrCpy(pInfo->m_Name, data.name, sizeof(pInfo->m_Name));
				pInfo->m_Type = FILE_TYPE;
				pInfo->m_Size = data.size;
				memcpy(&pInfo->m_Date, &data.time_write, 4);

				_findclose(handle);
				return true;
			}

			curRet = _findnext(handle, &data);
		}

		// Didn't find any files...
		_findclose(handle);
		return false;
	}
	else
	{
		pRezItm = pTree->m_pRezMgr->GetRezFromDosPath(pName);
		if (pRezItm != LTNULL)
		{
			LTStrCpy(pInfo->m_Name, pRezItm->GetName(), sizeof(pInfo->m_Name));
			pInfo->m_Type = FILE_TYPE;
			pInfo->m_Size = pRezItm->GetSize();
			pInfo->m_Date = pRezItm->GetTime();
			return true;
		}
		else
		{
			return false;
		}
	}
}


int df_GetDirInfo(HLTFileTree *hTree, char *pName)
{
	FileTree *pTree;
	_finddata_t data;
	long handle, curRet;
	char fullName[500];
	CRezDir* pRezDir;

	pTree = (FileTree*)hTree;
	if(!pTree)
		return 0;

	if(pTree->m_TreeType == DosTree)
	{
		LTSNPrintF(fullName, sizeof(fullName), "%s\\%s", pTree->m_BaseName, pName);

		handle = _findfirst(fullName, &data);
		curRet = handle;
		while(curRet != -1)
		{
			if(data.attrib & _A_SUBDIR)
			{
				_findclose(handle);
				return 1;
			}

			curRet = _findnext(handle, &data);
		}

		// Didn't find any.
		_findclose(handle);
		return 0;
	}
	else
	{
		pRezDir = pTree->m_pRezMgr->GetDirFromPath(pName);
		if (pRezDir != LTNULL) return 1;
		else return 0;
	}
}


int df_GetFullFilename(HLTFileTree *hTree, char *pName, char *pOutName, int maxLen)
{
	FileTree *pTree;

	pTree = (FileTree*)hTree;
	if(!pTree)
		return 0;

	if(pTree->m_TreeType != DosTree)
		return 0;
	
	LTSNPrintF(pOutName, maxLen, "%s\\%s", pTree->m_BaseName, pName);
	return 1;
}


ILTStream* df_Open(HLTFileTree *hTree, const char *pName, int openMode)
{
	FileTree *pTree = (FileTree*)hTree;
	if(!pTree)
		return LTNULL;

	if(pTree->m_TreeType == DosTree)
	{
		char fullName[500];
		LTSNPrintF(fullName, sizeof(fullName), "%s\\%s", pTree->m_BaseName, pName);
		
		FILE *fp;
		{
			CountAdder cntAdd(&g_PD_FOpen);
			fp = fopen(fullName, "rb");
		}
		if(!fp)
			return LTNULL;

		fseek(fp, 0, SEEK_END);
		uint32 fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// Use fp to setup the stream.
		DosFileStream *pDosStream = g_DosFileStreamBank.Allocate();
		pDosStream->m_pFile = fp;
		pDosStream->m_pTree = pTree;
		pDosStream->m_FileLen = fileLen;

		if (g_CV_ShowFileAccess >= 1)
		{
			dsi_ConsolePrint("stream %p open file %s size = %u",pDosStream,pName,fileLen);
		}

		return pDosStream;
	}
	else
	{
		CRezItm* pRezItm = pTree->m_pRezMgr->GetRezFromDosPath(pName);
		if (pRezItm == LTNULL) return LTNULL;

		// Use fp to setup the stream.
		RezFileStream *pRezStream = g_RezFileStreamBank.Allocate();
		pRezStream->m_pRezItm = pRezItm;
		pRezStream->m_pTree = pTree;
		pRezStream->m_FileLen = pRezItm->GetSize();
		pRezStream->m_SeekOffset = 0;

		if (g_CV_ShowFileAccess >= 1)
		{
			dsi_ConsolePrint("stream %p open rez %s size = %u",pRezStream,pName,pRezItm->GetSize());
		}

		return pRezStream;
	}
}


int df_FindNext(HLTFileTree *hTree, const char *pDirName, LTFindInfo *pInfo)
{
	LTFindData *pFindData;
	char filter[400];
	FileTree *pTree;
	long curRet;
	CRezItm* pRezItm;
	CRezDir* pRezDir;
	CRezTyp* pRezTyp;
	CRezDir* pRezSubDir;
	char	 sItemType[5];

	if(!hTree)
		return 0;

	pTree = (FileTree*)hTree;
	if(pTree->m_TreeType == DosTree)
	{
		if(!pInfo->m_pInternal)
		{
			LTSNPrintF(filter, sizeof(filter), "%s\\%s\\*.*", pTree->m_BaseName, pDirName);

			LT_MEM_TRACK_ALLOC(pFindData = (LTFindData*)dalloc(sizeof(LTFindData)),LT_MEM_TYPE_FILE);
			pFindData->m_pTree = pTree;
			pFindData->m_Handle = _findfirst(filter, &pFindData->m_Data);
			curRet = pFindData->m_Handle;

			pInfo->m_pInternal = pFindData;
		}
		else
		{
			pFindData = (LTFindData*)pInfo->m_pInternal;
			curRet = _findnext(pFindData->m_Handle, &pFindData->m_Data);
		}

		// Find a valid name..
		while(1)
		{
			if(curRet == -1)
			{
				pInfo->m_pInternal = LTNULL;

				if(pFindData->m_Handle != -1)
					_findclose(pFindData->m_Handle);
	
				dfree(pFindData);
				return 0;
			}

			if(pFindData->m_Data.name[0] == '.')
			{
				curRet = _findnext(pFindData->m_Handle, &pFindData->m_Data);
			}
			else
			{
				break;
			}
		}

		// Ok, found a valid one.
		LTStrCpy(pInfo->m_Name, pFindData->m_Data.name, sizeof(pInfo->m_Name));
		memcpy(&pInfo->m_Date, &pFindData->m_Data.time_write, 4);
		pInfo->m_Size = pFindData->m_Data.size;
		pInfo->m_Type = (pFindData->m_Data.attrib & _A_SUBDIR) ? DIRECTORY_TYPE : FILE_TYPE;

		return 1;
	}
	else
	{
		// If this is the first call to FindNext then set everything up
		if(pInfo->m_pInternal == LTNULL)
		{
			// find the directory
			pRezDir = pTree->m_pRezMgr->GetDirFromPath(pDirName);
			if(pRezDir == LTNULL)
			{
				return 0;
			}

			// get the first type
			pRezTyp = pRezDir->GetFirstType();
			if (pRezTyp != LTNULL)
			{
				// get the first item
				pRezItm = pRezDir->GetFirstItem(pRezTyp);
			}
			else 
			{ 
				pRezItm = LTNULL;
			}

			// if there were no files then get the first sub directory
			if (pRezItm == LTNULL)
			{
				pRezSubDir = pRezDir->GetFirstSubDir();
				if (pRezSubDir == LTNULL)
				{
					return 0;
				}
			}
			else pRezSubDir = LTNULL;

			// allocate the finddata structure
			LT_MEM_TRACK_ALLOC(pFindData = (LTFindData*)dalloc(sizeof(LTFindData)),LT_MEM_TYPE_FILE);
			pInfo->m_pInternal = pFindData;

			// fill in the members of the finddata structure
			pFindData->m_pTree = pTree;
			pFindData->m_pCurDir = pRezDir;
			pFindData->m_pCurTyp = pRezTyp;
			pFindData->m_pCurItm = pRezItm;
			pFindData->m_pCurSubDir = pRezSubDir;
		}

		// If this is not the first call then just get the next entry (or if at end clean up)
		else
		{
			pFindData = (LTFindData*)pInfo->m_pInternal;

			// are we searching for items
			if (pFindData->m_pCurItm != LTNULL)
			{
				pFindData->m_pCurItm = pFindData->m_pCurDir->GetNextItem(pFindData->m_pCurItm);
				
				// if we are finished with all the items of this type go to the next
				while(pFindData->m_pCurItm == LTNULL)
				{
					pFindData->m_pCurTyp = pFindData->m_pCurDir->GetNextType(pFindData->m_pCurTyp);

					// if we are done with all the types then we need to switch to checking sub directories
					if (pFindData->m_pCurTyp == LTNULL)
					{
						pFindData->m_pCurSubDir = pFindData->m_pCurDir->GetFirstSubDir();
						if (pFindData->m_pCurSubDir == LTNULL)
						{
							dfree(pInfo->m_pInternal);
							pInfo->m_pInternal = LTNULL;
							return 0;
						}
						break;
					}

					// get the first item of this type
					else pFindData->m_pCurItm = pFindData->m_pCurDir->GetFirstItem(pFindData->m_pCurTyp);
				}
			}

			// otherwise we are searching for sub directories
			else
			{
				pFindData->m_pCurSubDir = pFindData->m_pCurDir->GetNextSubDir(pFindData->m_pCurSubDir);
				if (pFindData->m_pCurSubDir == LTNULL)
				{
					dfree(pInfo->m_pInternal);
					pInfo->m_pInternal = LTNULL;
					return 0;
				}
			}
		}

		// is this a resource item
		if (pFindData->m_pCurItm != LTNULL)
		{
			LTStrCpy(pInfo->m_Name, pFindData->m_pCurItm->GetName(), sizeof(pInfo->m_Name)-4);
			pTree->m_pRezMgr->TypeToStr(pFindData->m_pCurItm->GetType(),sItemType);
			LTStrCat(pInfo->m_Name, ".", sizeof(pInfo->m_Name));
			LTStrCat(pInfo->m_Name, sItemType, sizeof(pInfo->m_Name));
			pInfo->m_Date = pFindData->m_pCurItm->GetTime();
			pInfo->m_Type = FILE_TYPE;
			pInfo->m_Size = pFindData->m_pCurItm->GetSize();
		}

		// is this a sub directory
		else if (pFindData->m_pCurSubDir != LTNULL)
		{
			LTStrCpy(pInfo->m_Name, pFindData->m_pCurSubDir->GetDirName(), sizeof(pInfo->m_Name));
			pInfo->m_Date = pFindData->m_pCurSubDir->GetTime();
			pInfo->m_Type = DIRECTORY_TYPE;
			pInfo->m_Size = 0;
		}

		else
		{
			return 0; // THIS SHOULD NEVER HAPPEN!
		}

		return 1;
	}
	
	return 0;
}


void df_FindClose(LTFindInfo *pInfo)
{
	LTFindData *pFindData;

	if(!pInfo || !pInfo->m_pInternal)
		return;

	pFindData = (LTFindData*)pInfo->m_pInternal;
	
	if(pFindData)
	{
		if(pFindData->m_pTree->m_TreeType == DosTree)
		{
			_findclose(pFindData->m_Handle);
		}

		dfree(pFindData);
		pInfo->m_pInternal = LTNULL;
	}
}


#define SAVEBUFSIZE 1024*16

// Save the contents of a steam to a file
// returns 1 if successful 0 if an error occured
int df_Save(ILTStream *hFile, const char *pName) {
	unsigned char* pSaveBuf;
	uint32 nBytesSaved;
	uint32 nAmountToRead, fileLen;
	FILE* pSaveFile;

	if(!hFile || hFile->ErrorStatus() != LT_OK)
		return 0;

	// open the save file for writing
	pSaveFile = fopen(pName, "wb");
	if (pSaveFile == LTNULL) return 0;

	// allocate memory for a temp buffer
	LT_MEM_TRACK_ALLOC(pSaveBuf = new uint8[SAVEBUFSIZE],LT_MEM_TYPE_FILE);
	if (pSaveBuf == LTNULL)
	{
		fclose(pSaveFile);
		return 0;
	}

	// set to the start of the stream
	hFile->SeekTo(0);

	// initialize number of bytes saved
	nBytesSaved = 0;

	// loop to copy entire file
	hFile->GetLen(&fileLen);
	while (nBytesSaved < fileLen)
	{
		// figure out how much data to read into the buffer
		nAmountToRead = fileLen - nBytesSaved;
		if (nAmountToRead > SAVEBUFSIZE) nAmountToRead = SAVEBUFSIZE;

		// read data from the stream
		hFile->Read(pSaveBuf, nAmountToRead);
		if (hFile->ErrorStatus() != LT_OK) break;

		// write out data to the save file and increment save counter
		if (fwrite(pSaveBuf, nAmountToRead, 1, pSaveFile) != 1) break;
		else nBytesSaved += nAmountToRead;
	}

	// clean up temp buffer
	delete [] pSaveBuf;

	// close the save file
	fclose(pSaveFile);

	// return correct success or fail status
	if (nBytesSaved != fileLen) return 0;
	else return 1;
};


// Returns raw file information for the file specified in pName found in the tree hTree
// sFileName returns the full name of the actual file that data is contained in 
//   if sFileName exceeds the size of nMaxFilename then an error is returned and the name is truncated
// sPos returns the position in the file that the data starts at
// nSize returns the size of the data 
int df_GetRawInfo(HLTFileTree *hTree, const char *pName, char* sFileName, unsigned int nMaxFileName, uint32* nPos, uint32* nSize)
{
	char fullName[500];
	FileTree *pTree;
	FILE *fp;
	CRezItm* pRezItm;

	// cast as a FileTree
	pTree = (FileTree*)hTree;
	if(!pTree) return 0;

	// if this is a dos file tree
	if(pTree->m_TreeType == DosTree)
	{
		// create the full name for the file
		LTSNPrintF(fullName, sizeof(fullName), "%s\\%s", pTree->m_BaseName, pName);

		// open the file to make sure it is ok
		fp = fopen(fullName, "rb");
		if(!fp)	return 0;

		// figure out the length of the file
		fseek(fp, 0, SEEK_END);
		*nSize = ftell(fp);

		// the position is the start of the file
		*nPos = 0;

		// close the file
		fclose(fp);

		// copy over the name 
		LTStrCpy(sFileName, fullName, nMaxFileName);

		// return error if name is too long
		if (nMaxFileName <= strlen(fullName)) return 0;

		// return LTTRUE everything went OK
		return 1;
	}
	else
	{
		// get the rez item that corresponds to this resource
		pRezItm = pTree->m_pRezMgr->GetRezFromDosPath(pName);
		if (pRezItm == LTNULL) return 0;

		// copy over the name 
		LTStrCpy(sFileName, pRezItm->DirectRead_GetFullRezName(), nMaxFileName);

		// return error if name is too long
		if (nMaxFileName <= strlen(pRezItm->DirectRead_GetFullRezName())) 
			return 0;

		// get the position of the resource in the resource file
		*nPos = pRezItm->DirectRead_GetFileOffset();

		// get the size of the resource
		*nSize = pRezItm->GetSize();

		// return LTTRUE everything went OK
		return 1;
	}
}


