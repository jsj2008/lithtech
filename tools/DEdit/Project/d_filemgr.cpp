//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#include "bdefs.h"
#include "d_filemgr.h"
#include "edithelpers.h"


// ------------------------------------------------------------- //
// Structures.
// ------------------------------------------------------------- //

typedef struct DFileMgr_t
{
	HHASHTABLE	m_DirIdentifiers;	// Directory identifiers, hashed on their (relative) names.
	HHASHTABLE	m_FileIdentifiers;	// File identifiers, hashed on their filenames.
	CString		m_BaseDir;			// Base directory.
} DFileMgr;


// ------------------------------------------------------------- //
// CFileIterator functions.
// ------------------------------------------------------------- //

static inline BOOL ShouldSkipCurrentFile(CFileIterator *pIterator)
{
	char *pFilename;

	pFilename = pIterator->m_Data.cFileName;
	if(pFilename[0] == 0)
	{
		return FALSE;
	}
	else
	{
		if(pFilename[0] == '.' && pFilename[1] == 0)
			return TRUE;
		else if(pFilename[0] == '.' && pFilename[1] == '.')
			return TRUE;
		else
			return FALSE;
	}
}

BOOL CFileIterator::Next(LPCTSTR pSearch, BOOL bRelative)
{
	if(m_Handle)
	{
		if(FindNextFile(m_Handle, &m_Data))
		{
			// Skip over the '.' and '..' directories.
			if(ShouldSkipCurrentFile(this))
				return Next(pSearch, bRelative);
			else
				return TRUE;
		}
		else
		{
			FindClose(m_Handle);
			m_Handle = NULL;
			return FALSE;
		}
	}
	else
	{
		CString fullSearchString;

		if(bRelative)
		{
			fullSearchString = dfm_GetFullFilename(GetFileMgr(), pSearch);
			pSearch = fullSearchString;
		}

		m_Handle = FindFirstFile(pSearch, &m_Data);
		if(m_Handle == INVALID_HANDLE_VALUE)
		{
			m_Handle = NULL;
			return FALSE;
		}
		else
		{
			// Skip over the '.' and '..' directories.
			if(ShouldSkipCurrentFile(this))
				return Next(pSearch, bRelative);
			else
				return TRUE;
		}
	}
}

void CFileIterator::Term()
{
	if(m_Handle)
	{
		FindClose(m_Handle);
		m_Handle = NULL;
	}
}



// ------------------------------------------------------------- //
// Interface functions.
// ------------------------------------------------------------- //

DFILEMGR dfm_Open(char *pBaseDir)
{
	DFileMgr *pRet;

	pRet = new DFileMgr;
	pRet->m_BaseDir = pBaseDir;
	pRet->m_DirIdentifiers = hs_CreateHashTable(500, HASH_STRING_NOCASE);
	pRet->m_FileIdentifiers = hs_CreateHashTable(500, HASH_STRING_NOCASE);

	return pRet;
}


void dfm_Close(DFILEMGR pMgr, IdentGoingAwayFn fn, void *pUser)
{
	HHASHITERATOR hIterator;
	HHASHELEMENT hElement;
	DFileIdent *pIdent;
	DDirIdent *pDirIdent;

	if(!pMgr)
		return;

	// Delete the FileIdents.
	hIterator = hs_GetFirstElement(pMgr->m_FileIdentifiers);
	while(hIterator)
	{
		hElement = hs_GetNextElement(&hIterator);
		
		pIdent = (DFileIdent*)hs_GetElementUserData(hElement);
		
		if(fn)
		{
			fn(pIdent, pUser);
		}

		delete pIdent;
	}

	// Delete the DirIdents.
	hIterator = hs_GetFirstElement(pMgr->m_DirIdentifiers);
	while(hIterator)
	{
		hElement = hs_GetNextElement(&hIterator);
		
		pDirIdent = (DDirIdent*)hs_GetElementUserData(hElement);
		delete pDirIdent;
	}

	hs_DestroyHashTable(pMgr->m_DirIdentifiers);
	hs_DestroyHashTable(pMgr->m_FileIdentifiers);
	delete pMgr;
}	


CString dfm_GetBaseDir(DFILEMGR pMgr)
{
	if(!pMgr)
		return "";

	return pMgr->m_BaseDir;
}


BOOL dfm_OpenFileRelative(DFILEMGR pMgr, LPCTSTR pFilename, CMoFileIO &file)
{
	char fullName[500];

	if(!pMgr)
		return FALSE;

	sprintf(fullName, "%s\\%s", pMgr->m_BaseDir, pFilename);
	return file.Open(fullName, "rb");
}


CString dfm_GetFullFilename(DFILEMGR pMgr, CString fileName)
{
	if(!pMgr)
		return fileName;

	return pMgr->m_BaseDir + "\\" + fileName;
}


int dfm_GetFileIdentifier(DFILEMGR pMgr, CString fileName, DFileIdent **pIdent)
{
	HANDLE hFile;
	HHASHELEMENT hElement;
	WIN32_FIND_DATA findData;
	DFileIdent *pRet;
	char fullName[500];

	*pIdent = NULL;

	ASSERT(pMgr);
	if(!pMgr)
		return DFM_NOFILE;

	// See if it's in the hash table.
	hElement = hs_FindElement(pMgr->m_FileIdentifiers, (void*)(LPCTSTR)fileName, fileName.GetLength()+1);
	if(hElement)
	{
		*pIdent = (DFileIdent*)hs_GetElementUserData(hElement);
		return DFM_OK;
	}

	// Ok, see if the file even exists.
	sprintf(fullName, "%s\\%s", (LPCTSTR)pMgr->m_BaseDir, (LPCTSTR)fileName);
	
	hFile = FindFirstFile(fullName, &findData);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return DFM_NOFILE;
	}
	else
	{
		FindClose(hFile);

		// Make a file identifier for the file.
		pRet = new DFileIdent;
		pRet->m_hElement = hs_AddElement(pMgr->m_FileIdentifiers, (void*)(LPCTSTR)fileName, fileName.GetLength()+1);
		hs_SetElementUserData(pRet->m_hElement, pRet);

		pRet->m_Filename = (char*)hs_GetElementKey(pRet->m_hElement, NULL);
		pRet->m_pUser = NULL;
		pRet->m_UserType = -1;

		*pIdent = pRet;
		return DFM_OK;
	}							  
}


int dfm_GetDirIdentifier(DFILEMGR pMgr, CString dirName, DDirIdent **pIdent)
{
	HANDLE hFile;
	HHASHELEMENT hElement;
	WIN32_FIND_DATA findData;
	DDirIdent *pRet;
	char fullName[500];
	DWORD attribs;

	*pIdent = NULL;

	ASSERT(pMgr);
	if(!pMgr)
		return DFM_NOFILE;

	// See if it's in the hash table.
	hElement = hs_FindElement(pMgr->m_DirIdentifiers, (void*)(LPCTSTR)dirName, dirName.GetLength()+1);
	if(hElement)
	{
		*pIdent = (DDirIdent*)hs_GetElementUserData(hElement);
		return DFM_OK;
	}

	// Ok, see if the file even exists.
	if(dirName.GetLength() == 0)
		strcpy(fullName, pMgr->m_BaseDir);
	else
		sprintf(fullName, "%s\\%s", (LPCTSTR)pMgr->m_BaseDir, (LPCTSTR)dirName);

	attribs = GetFileAttributes(fullName);
	if(attribs == 0xFFFFFFFF)
	{
		return DFM_NOFILE;
	}
	else if(attribs & FILE_ATTRIBUTE_DIRECTORY)
	{
		// Make a file identifier for the file.
		pRet = new DDirIdent;
		pRet->m_hElement = hs_AddElement(pMgr->m_DirIdentifiers, (void*)(LPCTSTR)dirName, dirName.GetLength()+1);
		hs_SetElementUserData(pRet->m_hElement, pRet);

		pRet->m_Filename = (char*)hs_GetElementKey(pRet->m_hElement, NULL);
		pRet->m_pUser = NULL;

		*pIdent = pRet;
		return DFM_OK;
	}
	else
	{
		return DFM_NOFILE;
	}
}



BOOL dfm_DoesFileExist(DFILEMGR pMgr, LPCTSTR fileName)
{
	HANDLE hFile;
	char fullName[500];
	WIN32_FIND_DATA findData;

	if(!pMgr)
		return FALSE;

	sprintf(fullName, "%s\\%s", (LPCTSTR)pMgr->m_BaseDir, (LPCTSTR)fileName);
	hFile = FindFirstFile(fullName, &findData);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	else
	{
		FindClose(hFile);
		return TRUE;
	}
}


CString dfm_BuildName(LPCTSTR pBaseName, LPCTSTR addOn)
{
	if(pBaseName[0] == 0)
		return addOn;
	else
		return CString(pBaseName) + "\\" + addOn;
}


void dfm_GetFileListWithUser(DFILEMGR pMgr, DLink *pListHead)
{
	DLink *pCur;
	HHASHITERATOR hIterator;
	HHASHELEMENT hElement;
	DFileIdent *pIdent;

	dl_TieOff(pListHead);
	if(!pMgr)
		return;

	hIterator = hs_GetFirstElement(pMgr->m_FileIdentifiers);
	while(hIterator)
	{
		hElement = hs_GetNextElement(&hIterator);
		pIdent = (DFileIdent*)hs_GetElementUserData(hElement);
		
		if(pIdent->m_pUser)
		{
			pCur = new DLink;
			pCur->m_pData = pIdent;
			dl_Insert(pListHead, pCur);
		}
	}
}


void dfm_FreeFileList(DFILEMGR hMgr, DLink *pListHead)
{
	DLink *pCur, *pNext;

	pCur = pListHead->m_pNext;
	while(pCur != pListHead)
	{
		pNext = pCur->m_pNext;
		delete pCur;
		pCur = pNext;
	}
}






