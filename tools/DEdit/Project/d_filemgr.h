//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

// This module defines dedit's file manager.  All file access 
// goes thru here.  The main task of dedit's file manager is to
// maintain the base directory and allow access to an identifier
// for each file (so data can be cached).

#ifndef __D_FILEMGR_H__
#define __D_FILEMGR_H__


	#include "dhashtable.h"


	// ------------------------------------------------------------- //
	// Structures.
	// ------------------------------------------------------------- //

	typedef struct DFileMgr_t *DFILEMGR;

	typedef struct DFileIdent_t
	{
		void			*m_pUser;		// Use this for whatever you want.
		int				m_UserType;		// Defaults to -1, 0 means a CTexture is in m_pUser.
										// 1 means a physics material display color is in m_pUser.

		HHASHELEMENT	m_hElement;		// Used internally.
		char			*m_Filename;	// Relative filename (including path).
	} DFileIdent;

	typedef struct DDirIdent_t
	{
		void			*m_pUser;		// Use this for whatever you want.
		HHASHELEMENT	m_hElement;		// Used internally.
		char			*m_Filename;	// (Relative) directory name.
	} DDirIdent;
		
	
	// This is a VERY helpful wrapper around FindFirstFile/FindNextFile/FindClose.
	// ALWAYS use it.  You can reuse a CFileIterator multiple times if you finish
	// iterating or call Term().  Always use the same pSearch while iterating.
	// An iteration looks like this:
	// CFileIterator iterator;
	// while(iterator.Next("*.*"))
	// {
	// 	...
	// }
	class CFileIterator
	{
		public:

					CFileIterator()		{m_Handle = NULL;}
					~CFileIterator()	{Term();}

			void	Term();

			BOOL	Next(LPCTSTR pSearch, BOOL bRelative);
			
			LPCTSTR	GetFilename()	{return m_Data.cFileName;}
			DWORD	GetAttributes()	{return m_Data.dwFileAttributes;}

			WIN32_FIND_DATA	m_Data;
			HANDLE			m_Handle;

	};


	// ------------------------------------------------------------- //
	// Defines.
	// ------------------------------------------------------------- //

	typedef void (*IdentGoingAwayFn)(DFileIdent *pIdent, void *pUser);


	// Status codes.
	#define DFM_OK		0
	#define DFM_NOFILE	1	// File doesn't exist.


	// ------------------------------------------------------------- //
	// Functions.
	// ------------------------------------------------------------- //

	// Opened and closed each time a project is opened/closed.
	DFILEMGR dfm_Open(char *pBaseDir);
	void dfm_Close(DFILEMGR hMgr, IdentGoingAwayFn fn, void *pUser);

	// Get the base directory filename (full name) for the filemgr.
	CString dfm_GetBaseDir(DFILEMGR hMgr);
	
	// Get the full filename (or directory name), given the relative name.
	CString dfm_GetFullFilename(DFILEMGR hMgr, CString fileName);

	// Open a file given a relative pathname.
	BOOL dfm_OpenFileRelative(DFILEMGR hMgr, LPCTSTR pFilename, CMoFileIO &file);

	// Get a file identifier.  Pass in a RELATIVE filename.
	// Returns a DFM_ error or DFM_OK.
	int dfm_GetFileIdentifier(DFILEMGR hMgr, CString fileName, DFileIdent **pIdent);

	// Get a directory identifier.  Pass in a RELATIVE name.
	int dfm_GetDirIdentifier(DFILEMGR hMgr, CString dirName, DDirIdent **pIdent);

	// Returns TRUE if the file exists (RELATIVE filename..)
	BOOL dfm_DoesFileExist(DFILEMGR hMgr, LPCTSTR fileName);

	// Helper function to add a filename/directory name to another.. automatically handles
	// it if the base filename is "".
	CString dfm_BuildName(LPCTSTR pBaseName, LPCTSTR addOn);


	// Returns a list of files with m_pUser set.  You supply the list head.
	void dfm_GetFileListWithUser(DFILEMGR hMgr, DLink *pListHead);

	void dfm_FreeFileList(DFILEMGR hMgr, DLink *pListHead);


#endif  // __D_FILEMGR_H__




