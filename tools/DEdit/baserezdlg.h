//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

// This module defines the CBaseRezDlg class, which all the resource 
// dialogs derive from.  

#ifndef __BASEREZDLG_H__
#define __BASEREZDLG_H__


	#include "resourcemgr.h"
	#include "d_filemgr.h"
	#include "mrcext.h"
	#include "projecttabcontrolbar.h"

	class CBaseRezDlg : public CProjectTabControlBar
	{
		public:

						DECLARE_DYNAMIC(CBaseRezDlg);

						CBaseRezDlg();
			
			void		InitBaseRezDlg(CString ext, CTreeCtrl *pTree, CListCtrl *pList, resource_type resType);
			CString		GetCurDirPath() {return m_csCurrentDir;}
			void		ClearAll();
			BOOL		IsDirectorySelected()	{return !!m_hCurrentItem;}

			DDirIdent*	GetFirstDirectory();
			DDirIdent*	GetSelectedDirectory();


		// Overrides.
		public:

			virtual void UpdateDirectories();	// Call when the directories change.
			virtual void PopulateList();		// Call to repopulate the list.
			
			// Called every time it adds a file to the file list.  The default sets
			// tab 1 to the file size and tab 2 to the last modified date.
			// The item data will have been set to the DFileIdent* already.
			virtual void SetListItemText(int nItem, CString &relativeFilename);

			void		OnOK( ) {}

		
		protected:
			
			// Adds a file to the list.  Must use a relative filename.
			int			AddFileToList(CString relativeFilename);
			int			FindFileInList(DFileIdent *pFileIdent );
			BOOL		SelectFileInList(DFileIdent *pFileIdent, BOOL bSelect = TRUE );
			
			// Returns TRUE if it ended up adding any directories.
			BOOL		RecurseAndAddDirs(HTREEITEM hParent);

			// This is called when the window is docked
			virtual void	OnSizedOrDocked(int cx, int cy, BOOL bFloating, int flags);

			// This is called to reposition the controls
			virtual void	RepositionControls();

		// Members.
		public:
			
			CString			m_Extension; // File extension of resource.
			CTreeCtrl		*m_pTree;
			CListCtrl		*m_pList;
			resource_type	m_ResourceType;

			CString		m_csCurrentDir;	// Current directory (relative pathname);
			HTREEITEM	m_hCurrentItem;
	
	};


#endif  // __BASEREZDLG_H__

