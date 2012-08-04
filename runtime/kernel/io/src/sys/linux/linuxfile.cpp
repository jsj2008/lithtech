// ------------------------------------------------------------------ //
// Includes..
// ------------------------------------------------------------------ //

#include <sys/stat.h>

#include "bdefs.h"
#include <string.h>
//#include <io.h>
#include <stdio.h>
#include <dirent.h>
#include "sysfile.h"
#include "de_memory.h"
#include "dutil.h"
//#include "dsys_interface.h"
#include "syslthread.h"
#include "syscounter.h"
#include "rezmgr.h"
#include "genltstream.h"

// console output of file access
// 0 - no output (default)
// 1 - display file open calls
// 2 - display file open and close calls
// 3 - display file open and close calls
// 4 - display file open and close and read calls
extern int32 g_CV_ShowFileAccess;

// PlayDemo profile info.
uint32 g_PD_FOpen=0;


typedef struct FileTree_t
{
	TreeType	m_TreeType;
	CRezMgr*	m_pRezMgr;
	char		m_BaseName[1];
} FileTree;


class BaseFileStream : public CGenLTStream
{
public:

		BaseFileStream()
		{
			m_FileLen = 0;
			m_SeekOffset = 0;
			m_pFile = LTNULL;
			m_pTree = LTNULL;
			m_ErrorStatus = 0;
			m_nNumReadCalls = 0;
			m_nTotalBytesRead = 0;
		}

	virtual	~BaseFileStream()
	{
		if (g_CV_ShowFileAccess >= 2)
		{
			dsi_ConsolePrint("close stream %p num reads = %u bytes read = %u", 
				this, m_nNumReadCalls, m_nTotalBytesRead);
		}

		if(m_pTree->m_TreeType == UnixTree && m_pFile)
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

	unsigned long	m_FileLen;		// Stored when the file is opened.
	unsigned long	m_SeekOffset;	// Seek offset (used in rezfiles) (NOTE: in a rezmgr rez file this is the current position inside the resource).
	FILE			*m_pFile;
	struct FileTree_t	*m_pTree;
	int				m_ErrorStatus;
	unsigned long	m_nNumReadCalls;
	unsigned long	m_nTotalBytesRead;
};


class UnixFileStream : public BaseFileStream
{
public:

	void	Release();

	LTRESULT GetPos(uint32 *pos)
	{
		*pos = (uint32)(ftell(m_pFile) - m_SeekOffset);
		return LT_OK;
	}
	
	LTRESULT SeekTo(uint32 offset)
	{
		long ret;

		ret = fseek(m_pFile, offset + m_SeekOffset, SEEK_SET);
		if(ret == 0)
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
		size_t sizeRead;

		if (g_CV_ShowFileAccess >= 4)
		{
			dsi_ConsolePrint("read %u bytes from stream %p",size, this);
		}

		if(m_ErrorStatus == 1)
		{
			memset(pData, 0, size);
			return LT_ERROR;
		}

		if(size != 0)
		{
			sizeRead = fread(pData, 1, size, m_pFile);
			if(sizeRead == size)
			{
				m_nNumReadCalls++;
				m_nTotalBytesRead += sizeRead;
				return LT_OK;
			}
			else
			{			
				memset(pData, 0, size);
				m_ErrorStatus = 1;
				return LT_ERROR;
			}
		}

		return LT_OK;
	}
};

static ObjectBank<UnixFileStream> g_UnixFileStreamBank(8, 8);


void UnixFileStream::Release()
{
	g_UnixFileStreamBank.Free(this);
}

// LTFindInfo::m_pInternal..
typedef struct LTFindData
{
	FileTree		*m_pTree;
	long			m_Handle;
};



// ------------------------------------------------------------------ //
// Interface functions.
// ------------------------------------------------------------------ //

void df_Init()
{
}

void df_Term()
{
}

int df_OpenTree(const char *pName, HLTFileTree *&pTreePointer)
{
	long handle, allocSize;
	FileTree *pTree;
	struct stat info;
	
	pTreePointer = NULL;

	// See if it exists..
	if (stat(pName,&info) != 0)
		return -1;

	allocSize = sizeof(FileTree) + strlen(pName);
	pTree = (FileTree*)dalloc_z(allocSize);
	pTree->m_TreeType = UnixTree;

	strcpy(pTree->m_BaseName, pName);
	pTreePointer = (HLTFileTree*)pTree;
	return 0;
}


void df_CloseTree(HLTFileTree* hTree)
{
	FileTree *pTree;

	pTree = (FileTree*)hTree;
	if(!pTree)
		return;
	dfree(pTree);
}


TreeType df_GetTreeType(HLTFileTree* hTree)
{
	FileTree *pTree;

	pTree = (FileTree*)hTree;
	if(!pTree)
		return UnixTree;

	return pTree->m_TreeType;
}


void df_BuildName(char *pPart1, char *pPart2, char *pOut)
{
	if(pPart1[0] == 0)
	{
		strcpy(pOut, pPart2);
	}
	else
	{
		sprintf(pOut, "%s/%s", pPart1, pPart2);
	}
}


int df_GetFileInfo(HLTFileTree* hTree, const char *pName, LTFindInfo *pInfo)
{
	FileTree *pTree;
	long handle, curRet;
	char fullName[500];
	CRezItm* pRezItm;

	pTree = (FileTree*)hTree;
	if(!pTree)
		return false;

	if(pTree->m_TreeType == UnixTree)
	{
		sprintf(fullName, "%s/%s", pTree->m_BaseName, pName);
		char* pLastSlash = strrchr(fullName, '/');
		*(pLastSlash++) = '\0';
		
		int numMatches;
		dirent** namelist;
		if ((numMatches = scandir(fullName, &namelist, 0, alphasort)) > 0)
		{
		        for (int iMatch = 0; iMatch < numMatches; ++iMatch)
			{
				if (namelist[iMatch]->d_type != DT_DIR) 
				{
					strncpy(pInfo->m_Name, namelist[iMatch]->d_name, 255);
					pInfo->m_Type = FILE_TYPE;
					//pInfo->m_Size = data.size;
					//memcpy(&pInfo->m_Date, &data.time_write, 4);
					return true;
				}
			}
		}

		// Didn't find any files...
		return false;
	}
	else
	{
		pRezItm = pTree->m_pRezMgr->GetRezFromDosPath(pName);
		if (pRezItm != LTNULL)
		{
			strncpy(pInfo->m_Name, pRezItm->GetName(), sizeof(pInfo->m_Name)-1);
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


int df_GetDirInfo(HLTFileTree hTree, char *pName)
{
	FileTree *pTree;
	char fullName[500];
	struct stat info;

	pTree = (FileTree*)hTree;
	if(!pTree) return 0;

	sprintf(fullName, "%s/%s", pTree->m_BaseName, pName);
	if (stat(fullName,&info) != 0) return 0;
	return (S_ISDIR(info.st_mode));
}


int df_GetFullFilename(HLTFileTree hTree, char *pName, char *pOutName, int maxLen)
{
	FileTree *pTree;

	pTree = (FileTree*)hTree;
	if(!pTree)
		return 0;

	if(pTree->m_TreeType != UnixTree)
		return 0;
	
	sprintf(pOutName, "%s/%s", pTree->m_BaseName, pName);
	return 1;
}


ILTStream* df_Open(HLTFileTree* hTree, const char *pName, int openMode)
{
	char fullName[500];
	FileTree *pTree;
	FILE *fp;
	UnixFileStream *pUnixStream;
	unsigned long seekOffset, fileLen;

	pTree = (FileTree*)hTree;
	if(!pTree)
		return NULL;

	if(pTree->m_TreeType != UnixTree) {
		return NULL;
	}

	sprintf(fullName, "%s/%s", pTree->m_BaseName, pName);
	CountAdder cntAdd(&g_PD_FOpen);
	if (! (fp = fopen(fullName, "rb")) )
		return NULL;

	fseek(fp, 0, SEEK_END);
	fileLen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	seekOffset = 0;

	// Use fp to setup the stream.
	pUnixStream = g_UnixFileStreamBank.Allocate();
	pUnixStream->m_pFile = fp;
	pUnixStream->m_pTree = pTree;
	pUnixStream->m_FileLen = fileLen;
	pUnixStream->m_SeekOffset = seekOffset;

	if (g_CV_ShowFileAccess >= 1)
	{
		dsi_ConsolePrint("stream %p open file %s size = %u",pUnixStream,pName,fileLen);
	}

	return pUnixStream;
}


int df_FindNext(HLTFileTree* hTree, const char *pDirName, LTFindInfo *pInfo)
{
	return 0;
}


void df_FindClose(LTFindInfo *pInfo)
{
}


#define SAVEBUFSIZE 1024*16

// Save the contents of a steam to a file
// returns 1 if successful 0 if an error occured
int df_Save(ILTStream *hFile, const char* pName)
{
	return 0;
}
