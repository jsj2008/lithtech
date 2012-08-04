//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditHelpers.h
//
//	PURPOSE	  : Defines lots of little helper functions and classes.
//
//	CREATED	  : December 14 1996
//
//
//------------------------------------------------------------------

#ifndef __EDITHELPERS_H__
	#define __EDITHELPERS_H__

	
	// Includes....
	#include "debugdlg.h"
	#include "propertiesdlg.h"
	#include "colorselectdlg.h"
	#include "dedit.h"


	
	class CProjectBar;
	class CEditProjectMgr;
	class CNodeView;
	class CRegionDoc;
	class ClassListDlg;
	class CTextureDlg;
	class CModelDlg;
	class CWorldsDlg;
	class CSoundDlg;
	class CSpriteDlg;
	class CPrefabDlg;
	class CLevelErrorDlg;
	class CLevelTexturesDlg;
	class CObjectSearchDlg;
	class CRenameResourceDlg;
	class CLevelItemsDlg;
	class CObjectSelFilterDlg;

	CDEditApp*			GetApp();

	
	// Management for the properties dlg.
	void				AddDebugMessage(char *pMsg, ...);
	
	CDebugDlg*			GetDebugDlg();
	CPropertiesDlg*		GetPropertiesDlg();
	ClassListDlg*		GetClassListDlg();
	CTextureDlg*		GetTextureDlg();
	CModelDlg*			GetModelDlg();
	CWorldsDlg*			GetWorldsDlg();
	CSoundDlg*			GetSoundDlg();
	CSpriteDlg*			GetSpriteDlg();
	CPrefabDlg*			GetPrefabDlg();

	CColorSelectDlg*	GetColorSelectDlg();
	CLevelErrorDlg*		GetLevelErrorDlg();
	CLevelItemsDlg*		GetLevelItemsDlg();
	CLevelTexturesDlg*	GetLevelTexturesDlg();
	CObjectSearchDlg*	GetObjectSearchDlg();
	CRenameResourceDlg* GetRenameResourceDlg();
	CNodeView *			GetNodeView( );
	CObjectSelFilterDlg*GetObjectSelFilterDlg();

	CMainFrame*			GetMainFrame();	
	CProjectBar*		GetProjectBar();
	CEditProjectMgr*	GetProject();
	struct DFileMgr_t*	GetFileMgr();
	
	// Get the active region doc (current doc in the foreground).
	CRegionDoc*			GetActiveRegionDoc();

	// Redraws all of the documents
	void				RedrawAllDocuments();

	//Gets which texture layer is currently being manipulated
	uint32				GetCurrTexture();
	void				SetCurrTexture(uint32 nCurrTex);

	// Displays a message box with the app's name in it.
	int					AppMessageBox( UINT idString, UINT nType );
	int					AppMessageBox( const char *pStr, UINT nType );

	inline BOOL IsKeyDown( int key )
	{
		return GetAsyncKeyState(key) & 0x8000;
	}

	class CHelperFileDlg : public CFileDialog
	{
		public:

							CHelperFileDlg(	
									BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL, 
									DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, 
									CWnd* pParentWnd = NULL  );

			int				DoModal();
			
	
		private:

			char			m_szFileName[50000];

	};




#endif  // __EDITHELPERS_H__

